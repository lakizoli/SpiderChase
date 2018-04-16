/*!
\brief INTERNAL TO RenderManager. Implementation of the EffectApi class.
\file PVRUtils/Vulkan/EffectVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRUtils/Vulkan/EffectVk.h"
#include "PVRVk/ApiObjectsVk.h"
#include "PVRCore/IO/BufferStream.h"
#include "PVRVk/SwapchainVk.h"
#include "HelperVk.h"
namespace pvr {
namespace effect {
namespace impl {
using namespace pvrvk;
namespace {
std::map<Api, std::set<StringHash>/**/> apiToStringList;
inline void addMapping(Api api, const StringHash& str)
{
	apiToStringList[api].insert(str);
}

inline bool initializeStringLists()
{
	apiToStringList.clear();
	addMapping(Api::Vulkan, "Vulkan");
	addMapping(Api::Vulkan, "vulkan");
	addMapping(Api::Vulkan, "VK");
	addMapping(Api::Vulkan, "vk");
	addMapping(Api::Vulkan, "VULKAN");
	addMapping(Api::OpenGLES2, "OpenGLES2");
	addMapping(Api::OpenGLES2, "OGLES2");
	addMapping(Api::OpenGLES2, "GLES2");
	addMapping(Api::OpenGLES2, "GL2");
	addMapping(Api::OpenGLES3, "OpenGLES3");
	addMapping(Api::OpenGLES3, "OGLES3");
	addMapping(Api::OpenGLES3, "GLES3");
	addMapping(Api::OpenGLES3, "GL3");
	addMapping(Api::OpenGLES31, "OpenGLES31");
	addMapping(Api::OpenGLES31, "OGLES31");
	addMapping(Api::OpenGLES31, "GLES31");
	addMapping(Api::OpenGLES31, "GL31");
	return true;
}

inline StringHash findMatchingApiString(const Effect_::AssetEffect& asset, Api api)
{
	StringHash retval("");
	const auto& versions = asset.getVersions();

	auto apiStringsIt = apiToStringList.find(api);
	if (apiStringsIt == apiToStringList.end())
	{
		Log(LogLevel::Warning, "EffectApi: Could not find any matching std::string in the Effect asset ('apiversion' elements), so will use the default (empty std::string) implementation.\n"
		    "If the effect has not been designed to work with OpenGL ES implementations in the default settings, errors will occur.\n"
		    "Default strings that OpenGL ES implementations accept are :\n"
		    "[nothing], 'OpenGLES2', 'OGLES2', 'GLES2', 'GL2','OpenGLES3', 'OGLES3', 'GLES3', 'GL3','OpenGLES31', 'OGLES31', 'GLES31', 'GL31'"
		    "Default strings that Vulkan implementations accept are :\n"
		    "[nothing], 'VK', 'vk', 'VULKAN', 'Vulkan', 'vulkan'"
		    "If providing an apiversion other than this, use the function pvr::utils::effect::addApiversionStringMapping from application code"
		    "to add it to your implementation."
		   );
		return retval;
	}

	Api invalidVersion = api > Api::OpenGLESMaxVersion ? Api::OpenGLESMaxVersion : Api::Unspecified;
	while (api > invalidVersion)
	{
		apiStringsIt = apiToStringList.find(api);
		for (auto versionsit = versions.begin(); versionsit != versions.end(); ++versionsit)
		{
			for (auto apiStrings = apiStringsIt->second.begin(); apiStrings != apiStringsIt->second.end(); ++apiStrings)
			{
				if (*versionsit == *apiStrings)
				{
					retval = *apiStrings;
					return retval;
				}
			}
		}
		api = Api((int)api - 1);
	}
	return retval;
}

inline bool addTexture(Effect_& effect,
                       const assets::effect::TextureReference& textureRef,
                       std::map<StringHash, ImageView>& textures, const StringHash& pipeline,
                       CommandBuffer& cmdBuffer, IAssetProvider& assetProvider,
                       std::vector<utils::ImageUploadResults>& uploadResults)
{
	PipelineDef* pipeDef =  effect.getPipelineDefinition(pipeline);
	if (!pipeDef) { return false; }
	pipeDef->descSetExists[textureRef.set] = true;
	ImageView view;
	Device device = effect.getDevice()->getReference();
	if (!textureRef.textureName.empty())
	{
		const assets::effect::TextureDefinition& textureDef =
		  effect.getEffectAsset().textures.find(textureRef.textureName)->second;

		if (textureDef.path.length())
		{
			uploadResults.push_back(
			  utils::loadAndUploadImage(device,
			                            textureDef.path.c_str(), true, cmdBuffer, assetProvider));
		}
		else
		{
			VkFormat format = (VkFormat)utils::convertToVk(textureDef.fmt);
			Image image = utils::createImage(device, VkImageType::e_2D, format,
			                                  pvrvk::Extent3D(textureDef.width, textureDef.height, 1u),
			                                  VkImageUsageFlags::e_SAMPLED_BIT);
			if (image.isValid())
			{
				view = device->createImageView(image);
			}
		}

		if (view.isNull())
		{
			Log("ApiEffect: Failed to create texture with name %s", textureDef.name.c_str());
			return false;
		}
		else
		{
			textures[textureDef.name] = view;
		}
	}
	else if (!textureRef.semantic.empty())
	{
		pipeDef->descSetIsFixed[textureRef.set] = false;
		effect.registerTextureSemantic(pipeline, textureRef.semantic, textureRef.set, textureRef.binding);
	}
	else
	{
		Log("ApiEffect: For pipeline [%s] texture [%s] neither a 'name' nor a "
		    "'semantic' attribute was not defined. If this texture is to be loaded"
		    " or created by the effect, define a <texture> element in the pfx file"
		    " and set the 'name' attribute in the pipeline's <texture> element. "
		    "If this texture is intended will be provided with"
		    "a model, define the 'semantic' attribute in the ");
		return false;
	}
	return true;
}

inline bool addBuffer(Effect_& effect, effect::PipelineDef& pipedef,
                      const assets::effect::BufferRef& bufferRef, std::map<StringHash, BufferDef>& buffers,
                      uint32_t swapchainLength)
{
	pipedef.descSetExists[bufferRef.set] = true;
	if (bufferRef.bufferName.length())
	{
		const assets::effect::BufferDefinition& assetBufferDef =
		  effect.getEffectAsset().buffers.find(bufferRef.bufferName)->second;

		BufferDef& bufferdef = buffers[assetBufferDef.name];

		pipedef.descSetIsMultibuffered[bufferRef.set] = assetBufferDef.multibuffering;
		if (bufferdef.scope == VariableScope::Unknown) // First time this buffer is being referenced.
		{
			assertion(bufferdef.buffer.isNull());

			bufferdef.allSupportedBindings = assetBufferDef.allSupportedBindings;
			bufferdef.isDynamic = assetBufferDef.isDynamic;
			bufferdef.scope = assetBufferDef.scope;
			bufferdef.memoryDescription.setName(bufferRef.bufferName.c_str());
			for (size_t i = 0; i < assetBufferDef.entries.size(); ++i)
			{
				bufferdef.memoryDescription.addElement(assetBufferDef.entries[i].semantic,
				                                       assetBufferDef.entries[i].dataType, assetBufferDef.entries[i].arrayElements);
			}

			if (assetBufferDef.multibuffering)
			{
				bufferdef.numBuffers = static_cast<uint16_t>(swapchainLength);
			}
		}

		//Add it to the pipeline's lists
		switch (bufferdef.scope)
		{
		case VariableScope::Effect:
		{
			auto& binfo = pipedef.effectScopeBuffers[assetBufferDef.name];
			static_cast<BufferRef&>(binfo) = static_cast<const BufferRef&>(bufferRef);
		}
		break;
		case VariableScope::Model:
		case VariableScope::BoneBatch:
		{
			pipedef.descSetIsFixed[bufferRef.set] = false;

			auto& binfo = pipedef.modelScopeBuffers[assetBufferDef.name];
			static_cast<BufferRef&>(binfo) = static_cast<const BufferRef&>(bufferRef);
		}
		break;
		case VariableScope::Node:
		{
			pipedef.descSetIsFixed[bufferRef.set] = false;
			auto& binfo = pipedef.nodeScopeBuffers[assetBufferDef.name];
			static_cast<BufferRef&>(binfo) = static_cast<const BufferRef&>(bufferRef);
		}
		break;
		}
	}
	else
	{
		Log("ApiEffect: A buffer with name [%s] was not properly defined but "
		    "was referenced in a pipeline", bufferRef.bufferName.c_str());
		return false;
	}
	return true;
}

inline void createTextures(
  Effect_& effect, std::map<StringHash, ImageView>& textures,
  CommandBuffer& uploadCmdBuffer, IAssetProvider& assetProvider,
  std::vector<utils::ImageUploadResults>& uploadResults)
{
	const Effect_::AssetEffect assetEffect = effect.getEffectAsset();
	auto pipes_it = assetEffect.versionedPipelines.find(effect.getApiString());
	if (pipes_it != assetEffect.versionedPipelines.end())
	{
		auto& pipes = pipes_it->second;
		for (auto pipe_it = pipes.begin(); pipe_it != pipes.end(); ++pipe_it)
		{
			for (auto texture_it = pipe_it->second.textures.begin();
			     texture_it != pipe_it->second.textures.end(); ++texture_it)
			{
				addTexture(effect, *texture_it, textures, pipe_it->first,
				           uploadCmdBuffer, assetProvider, uploadResults);
			}
		}
	}
}

inline void createBuffers(Effect_& effect,
                          std::map<StringHash, effect::PipelineDef>& createParams,
                          std::map<StringHash, BufferDef>& buffers, uint32_t swapchainLength)
{
	const Effect_::AssetEffect assetEffect = effect.getEffectAsset();
	auto pipes = assetEffect.versionedPipelines.find(effect.getApiString());
	if (pipes != assetEffect.versionedPipelines.end())
	{
		for (auto pipe_it = pipes->second.begin(); pipe_it != pipes->second.end(); ++pipe_it)
		{
			auto pipeDef = effect.getPipelineDefinition(pipe_it->first);
			if (!pipeDef) { continue; }
			for (auto buffer_it = pipe_it->second.buffers.begin();
			     buffer_it != pipe_it->second.buffers.end(); ++buffer_it)
			{
				addBuffer(effect, *pipeDef, *buffer_it, buffers, swapchainLength);
			}
		}
	}
}

const assets::effect::PipelineDefinition* getPipeline(
  const assets::effect::Effect& effect,
  const StringHash& version, const StringHash& name)
{
	const Effect_::AssetEffect& assetEffect = effect;
	auto pipes = assetEffect.versionedPipelines.find(version);
	if (pipes == assetEffect.versionedPipelines.end())
	{
		pipes = assetEffect.versionedPipelines.find(StringHash());
	}
	if (pipes != assetEffect.versionedPipelines.end())
	{
		auto pipe = pipes->second.find(name);
		if (pipe != pipes->second.end())
		{
			return  &pipe->second;
		}
	}
	return NULL;
}

bool getRenderPassAndFramebufferForPass(Effect_& effect, const assets::effect::Effect& effectAsset,
                                        const assets::effect::Pass& pass, Framebuffer* framebuffers, RenderPass& rp,
                                        std::vector<std::pair<StringHash, uint32_t>/**/>& colorAttachmentIndex)
{
	DeviceWeakPtr device = effect.getDevice();
	FramebufferCreateInfo framebufferInfo[FrameworkCaps::MaxSwapChains];
	RenderPassCreateInfo rpInfo;
	Swapchain swapchain = effect.getSwapchain();
	{
		std::set<assets::effect::TextureDefinition> colorAttachmentsSet;
		std::set<assets::effect::TextureDefinition> inputAttachments;
		int32_t swapchainTargetIndex = -1;
		// gather all sub passes targets and input attachments
		// keep a unique list of tarets  and input attachments from the subpass
		for (uint32_t i = 0; i < pass.subpasses.size(); ++i)
		{
			const assets::effect::Subpass& subpass = pass.subpasses[i];

			// TARGET ATTACHMENTS

			for (uint32_t target = 0; target < ARRAY_SIZE(subpass.targets); ++target)
			{
				if (!subpass.targets[target].empty())
				{
					if (subpass.targets[target] == "default") // Is this Swapchain image
					{
						swapchainTargetIndex = target;
					}
					else
					{
						const auto& found = effectAsset.textures.find(subpass.targets[target]);
						if (found != effectAsset.textures.end())
						{
							colorAttachmentsSet.insert(found->second);
						}
					}
				}
			}

			// INPUT ATTACHMENTS
			for (uint32_t input  = 0; input < ARRAY_SIZE(subpass.inputs); ++input)
			{
				if (!subpass.inputs[input].empty())
				{
					const auto& found = effectAsset.textures.find(subpass.inputs[input]);
					if (found != effectAsset.textures.end())
					{
						inputAttachments.insert(found->second);
					}

				}
			}
		}// next subpass

		// assign the unique list of input attachment to the vector for indexing
		// if it is onscreen framebuffer then make the attachment 0 is the swapchain image
		std::vector<assets::effect::TextureDefinition> colorAttachments(swapchainTargetIndex != -1 ?
		    colorAttachmentsSet.size() + 1 : colorAttachmentsSet.size());

		const uint32_t framebufferWidth = swapchain->getDimension().width;
		const uint32_t framebufferHeight =  swapchain->getDimension().height;
		if (swapchainTargetIndex != -1)
		{
			colorAttachments[0] = assets::effect::TextureDefinition("default", "",
			                      framebufferWidth, framebufferHeight, ImageDataFormat());// add a dummy object here.
		}
		std::copy(colorAttachmentsSet.begin(), colorAttachmentsSet.end(),
		          swapchainTargetIndex != -1 ? colorAttachments.begin() + 1 : colorAttachments.begin());
		colorAttachmentsSet.clear();

		// if it is swapchain and the target index is not 0 then swap so that they are in the right
		// order in the list.
		if (swapchainTargetIndex != -1 && swapchainTargetIndex != 0)
		{
			std::swap(colorAttachments[swapchainTargetIndex], colorAttachments[0]);
		}

		for (uint32_t i = 0; i < colorAttachments.size(); ++i)
		{
			const assets::effect::TextureDefinition& texDef = colorAttachments[i];
			// make sure the attachments image's width and height are consistent
			if (framebufferWidth != texDef.width)
			{
				Log(LogLevel::Warning, "Framebuffer attachment %s width is inconsistent "
				    "with other attachments. Forcing to %d", texDef.name.c_str(), framebufferWidth);
			}
			if (framebufferHeight != texDef.height)
			{
				Log(LogLevel::Warning, "Framebuffer attachment %s height is inconsistent "
				    "with other attachments. Forcing to %d", texDef.name.c_str(), framebufferHeight);
			}

			for (uint32_t swapChainId = 0; swapChainId < swapchain->getSwapchainLength(); ++swapChainId)
			{
				Image texture;

				// Create on Screen framebuffer attachment
				// attachment index 0 is reserved for the swapchain image

				// if it is the first attachment then it has to be swapchain image.
				// else if it is not the first attachment then  create a transient attachment if the
				// subpasses use them as input and targets. Transient attachments are lazly allocated on the
				// device and the memory is optimized out on some driver.
				if (static_cast<int32_t>(i) == swapchainTargetIndex)
				{
					rpInfo.setAttachmentDescription(i, AttachmentDescription::createColorDescription(
					                                  swapchain->getImageFormat()));
					framebufferInfo[swapChainId].setAttachment(i, swapchain->getImageView(swapChainId));
				}
				else
				{
					VkFormat fmt = (VkFormat)utils::convertToVk(texDef.fmt);
					auto found = std::find_if(inputAttachments.begin(), inputAttachments.end(), [&](const assets::effect::TextureDefinition & texDefinition)
					{
						return (texDefinition.name == texDef.name);
					});

					VkAttachmentStoreOp storeOp = VkAttachmentStoreOp::e_STORE;

					if (found != inputAttachments.end())
					{
						// create the transient image
						texture = utils::createImage(device, VkImageType::e_2D, fmt, pvrvk::Extent3D(framebufferWidth, framebufferHeight, 1u),
						                              VkImageUsageFlags::e_COLOR_ATTACHMENT_BIT | VkImageUsageFlags::e_INPUT_ATTACHMENT_BIT |
						                              VkImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT, VkImageCreateFlags(0), pvrvk::ImageLayersSize(),
						                              VkSampleCountFlags::e_1_BIT, VkMemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT);
						storeOp = VkAttachmentStoreOp::e_DONT_CARE;
					}
					else
					{
						texture = utils::createImage(device, VkImageType::e_2D, fmt, pvrvk::Extent3D(framebufferWidth, framebufferHeight, 1u),
						                              VkImageUsageFlags::e_COLOR_ATTACHMENT_BIT);
					}
					framebufferInfo[swapChainId].setAttachment(i, device->createImageView(texture));
					rpInfo.setAttachmentDescription(i, AttachmentDescription::createColorDescription(fmt, VkImageLayout::e_UNDEFINED,
					                                VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, VkAttachmentLoadOp::e_CLEAR, storeOp));
					colorAttachmentIndex.push_back(std::make_pair(texDef.name, i));
				}

			}
		}// next color attachment

		//------------------------------------
		// Depth stencil attachment
		uint32_t depthStencilAttachmentIndex = static_cast<uint32_t>(-1);
		ImageView dsAttachments[FrameworkCaps::MaxSwapChains];
		if (!pass.targetDepthStencil.empty()) // depth stencil
		{
			const auto& found =  effectAsset.textures.find(pass.targetDepthStencil);
			if (found != effectAsset.textures.end())
			{
				if (framebufferWidth != found->second.width)
				{
					Log(LogLevel::Warning, "Framebuffer attachment %s width is inconsistent with other attachments. "
					    "Forcing to %d", found->first.c_str(), framebufferWidth);
				}
				if (framebufferHeight != found->second.height)
				{
					Log(LogLevel::Warning, "Framebuffer attachment %s height is inconsistent with other attachments. "
					    "Forcing to %d", found->first.c_str(), framebufferHeight);
				}
				VkFormat format = utils::convertToVk(found->second.fmt);
				for (uint32_t ii = 0; ii < swapchain->getSwapchainLength(); ++ii)
				{
					Image tex = utils::createImage(device, VkImageType::e_2D, format, pvrvk::Extent3D(framebufferWidth, framebufferHeight, 1u),
					                                VkImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | VkImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT,
					                                VkImageCreateFlags(0), pvrvk::ImageLayersSize(), VkSampleCountFlags::e_1_BIT, VkMemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT);
					dsAttachments[ii] = device->createImageView(tex);
				}
				depthStencilAttachmentIndex = rpInfo.getNumAttachmentDescription();
				rpInfo.setAttachmentDescription(depthStencilAttachmentIndex,
				                                AttachmentDescription::createDepthStencilDescription(format, VkImageLayout::e_UNDEFINED));
			}
			else
			{
				Log("EffectApi: Depth-Stencil attachment referenced in pass %s is not found");
			}
		}

		//-----------------------
		// create the subpass
		for (uint32_t i = 0; i < pass.subpasses.size(); ++i)
		{
			SubPassDescription subpass;
			uint8_t attachmentId = 0, preserveAttachmentIndex = 0;
			// for each color attachments:
			// add the attachment as a color / input attachment if the sub pass uses them.
			for (auto it = colorAttachments.cbegin(); it != colorAttachments.cend(); ++it, ++attachmentId)
			{
				if (pass.subpasses[i].targets[0] == it->name)
				{
					subpass.setColorAttachmentReference(0, AttachmentReference(attachmentId, VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
				}
				else if (pass.subpasses[i].targets[1] == it->name)
				{
					subpass.setColorAttachmentReference(1, AttachmentReference(attachmentId, VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
				}
				else if (pass.subpasses[i].targets[2] == it->name)
				{
					subpass.setColorAttachmentReference(2, AttachmentReference(attachmentId, VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
				}
				else if (pass.subpasses[i].targets[3] == it->name)
				{
					subpass.setColorAttachmentReference(3, AttachmentReference(attachmentId, VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
				}
				else if (pass.subpasses[i].inputs[0] == it->name)
				{
					subpass.setInputAttachmentReference(0, AttachmentReference(attachmentId, VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
					++preserveAttachmentIndex;
				}
				else if (pass.subpasses[i].inputs[1] == it->name)
				{
					subpass.setInputAttachmentReference(1, AttachmentReference(attachmentId, VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
					++preserveAttachmentIndex;
				}
				else if (pass.subpasses[i].inputs[2] == it->name)
				{
					subpass.setInputAttachmentReference(2, AttachmentReference(attachmentId, VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
					++preserveAttachmentIndex;
				}
				else if (pass.subpasses[i].inputs[3] == it->name)
				{
					subpass.setInputAttachmentReference(3, AttachmentReference(attachmentId, VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
					++preserveAttachmentIndex;
				}
				else
				{
					subpass.setPreserveAttachmentReference(preserveAttachmentIndex, attachmentId);
					++preserveAttachmentIndex;
				}
			}

			if (pass.subpasses[i].useDepthStencil && depthStencilAttachmentIndex != static_cast<uint32_t>(-1))
			{
				subpass.setDepthStencilAttachmentReference(AttachmentReference(depthStencilAttachmentIndex,
				    VkImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));
			}

			rpInfo.setSubPass(i, subpass);
		}// next subpass

		//-----------------------------------------
		// Subpass dependency
		// loop through all subppasses and create a dependecy chain between them.
		for (uint32_t i = 0; i < rpInfo.getNumSubPasses(); ++i)
		{
			const SubPassDescription& subpassDst = rpInfo.getSubPass(i);
			for (uint32_t j = 0; j < rpInfo.getNumSubPasses(); ++j)
			{
				// avoid if src and dest subpasses are same and the src sub pass index must be less than the
				// destination sub pass index
				if (j >= i) { continue; }
				const SubPassDescription& subpassSrc = rpInfo.getSubPass(j);

				auto  srcAccessFlag = VkAccessFlags(0);
				auto  dstAccessFlag = VkAccessFlags(0);

				// COLOR
				if (subpassSrc.getNumColorAttachmentReference() != 0 && subpassDst.getNumInputAttachmentReference() != 0)
				{
					srcAccessFlag |= VkAccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT;
					dstAccessFlag |= VkAccessFlags::e_COLOR_ATTACHMENT_READ_BIT;
				}
				else if (subpassSrc.getNumColorAttachmentReference() != 0 && subpassDst.getNumColorAttachmentReference() != 0)
				{
					srcAccessFlag |= VkAccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT;
					dstAccessFlag |= VkAccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT;
				}
				else if (subpassSrc.getNumInputAttachmentReference() != 0 &&
				         subpassDst.getNumInputAttachmentReference() != 0)
				{
					srcAccessFlag |= VkAccessFlags::e_COLOR_ATTACHMENT_READ_BIT;
					dstAccessFlag |= VkAccessFlags::e_COLOR_ATTACHMENT_READ_BIT;
				}

				// DEPTH STENCIL
				if (subpassSrc.getDepthStencilAttachmentReference().layout != VkImageLayout::e_UNDEFINED &&
				    subpassDst.getDepthStencilAttachmentReference().layout != VkImageLayout::e_UNDEFINED)
				{
					srcAccessFlag |= VkAccessFlags::e_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					dstAccessFlag |= VkAccessFlags::e_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				}
				else if (subpassSrc.getDepthStencilAttachmentReference().layout != VkImageLayout::e_UNDEFINED)
				{
					srcAccessFlag |= VkAccessFlags::e_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					dstAccessFlag |= VkAccessFlags::e_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
				}
				else if (subpassDst.getDepthStencilAttachmentReference().layout != VkImageLayout::e_UNDEFINED)
				{
					srcAccessFlag |= VkAccessFlags::e_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
					dstAccessFlag |= VkAccessFlags::e_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				}

				rpInfo.addSubPassDependency(SubPassDependency(j, i, VkPipelineStageFlags::e_ALL_GRAPHICS_BIT,
				                            VkPipelineStageFlags::e_ALL_GRAPHICS_BIT, srcAccessFlag, dstAccessFlag, VkDependencyFlags::e_BY_REGION_BIT));
			}
		}

		rp = device->createRenderPass(rpInfo);
		if (!rp.isValid())
		{
			Log("EffectApi: Failed to create a renderpass for the pass: %s", pass.name.c_str());
			return false;
		}

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			framebufferInfo[i].width = framebufferWidth;
			framebufferInfo[i].height = framebufferHeight;
			framebufferInfo[i].renderPass = rp;
			if (dsAttachments[i].isValid())
			{
				framebufferInfo[i].setAttachment(depthStencilAttachmentIndex, dsAttachments[i]);
			}
			framebuffers[i] = device->createFramebuffer(framebufferInfo[i]);
		}
		return framebuffers[0].isValid();
	}
	return false;
}

void createPasses(Effect_& effect, std::vector<Pass>& passes, std::map<StringHash, PipelineLayout>& layouts,
                  std::map<StringHash, effect::PipelineDef>& createParams, std::map<StringHash,
                  std::map<StringHash, effect::TextureInfo>/**/>& samplersIndexedByPipeAndTexture, uint32_t swapchainLength)
{
	const assets::effect::Effect& assetEffect = effect.getEffectAsset();
	uint32_t pass_idx = 0;

	passes.resize(assetEffect.passes.end() - assetEffect.passes.begin());

	// for each pass
	for (auto pass_it = assetEffect.passes.begin(); pass_it != assetEffect.passes.end(); ++pass_it, ++pass_idx)
	{
		Pass& pass = passes[pass_idx];
		// The color index keep an index in to the framebuffer's color attachments. will be used later
		std::vector<std::pair<StringHash, uint32_t>/**/> colorIndex;
		getRenderPassAndFramebufferForPass(effect, assetEffect, *pass_it, pass.framebuffers, pass.renderPass, colorIndex);

		uint32_t subpass_idx = 0;
		pass.subpasses.resize(static_cast<uint32_t>((pass_it->subpasses.end() - pass_it->subpasses.begin())));
		// for each subpasses within the pass

		for (auto subpass_it = pass_it->subpasses.begin(); subpass_it != pass_it->subpasses.end(); ++subpass_it, ++subpass_idx)
		{
			auto subpassGroupId = static_cast<uint32_t>(0);
			Subpass& subpass = pass.subpasses[subpass_idx];
			subpass.groups.resize(static_cast<uint32_t>((subpass_it->groups.end() - subpass_it->groups.begin())));
			for (auto supassGroup_it = subpass_it->groups.begin(); supassGroup_it != subpass_it->groups.end();
			     ++supassGroup_it, ++subpassGroupId)
			{
				uint32_t pipe_idx = 0;
				SubpassGroup& group = subpass.groups[subpassGroupId];
				group.name = supassGroup_it->name;
				group.pipelines.resize(static_cast<uint32_t>((supassGroup_it->pipelines.end() - supassGroup_it->pipelines.begin())));
				for (auto pipe_it = supassGroup_it->pipelines.begin(); pipe_it != supassGroup_it->pipelines.end();
				     ++pipe_it, ++pipe_idx)
				{
					ConditionalPipeline& pipeline = group.pipelines[pipe_idx];
					pipeline.conditions = pipe_it->conditions;
					pipeline.identifiers = pipe_it->identifiers;

					const assets::effect::PipelineDefinition* pipedef = getPipeline(assetEffect, effect.getApiString(), pipe_it->pipelineName);

					if (pipedef == NULL)
					{
						Log("EffectApi initialization: Could not find the pipeline [%s] referenced in pass #%d subpass #%d",
						    pipe_it->pipelineName.c_str(), pass_idx, subpass_idx);
						continue;
					}
					effect::PipelineDef& effectPipeDef = createParams[pipedef->name];
					GraphicsPipelineCreateInfo& cp = effectPipeDef.createParam;

					pipeline.pipeline = pipedef->name;

					// Vertex Attributes
					effectPipeDef.attributes = pipedef->attributes;

					// INPUT Attachments
					for (uint32_t i = 0; i < assets::effect::Subpass::MaxInputs; ++i)
					{
						const StringHash& name = subpass_it->inputs[i];
						if (!name.empty())
						{
							// get the location where the attachment is in the framebuffer
							std::vector<std::pair<StringHash, uint32_t>/**/>::iterator it =
							  std::find_if(colorIndex.begin(), colorIndex.end(),
							[&name](std::pair<StringHash, uint32_t>& attachment) {return name == attachment.first;});

							for (uint32_t j = 0; j < swapchainLength; ++j)
							{
								std::vector<assets::effect::InputAttachmentRef>::const_iterator assetAtatchmentRef =
								  std::find_if(pipedef->inputAttachments.begin(), pipedef->inputAttachments.end(),
								[ = ](const assets::effect::InputAttachmentRef & ref) { return ref.targetIndex == static_cast<int8_t>(i); });
								if (assetAtatchmentRef != pipedef->inputAttachments.end())
								{
									effectPipeDef.inputAttachments[j][name] =
									  InputAttachmentInfo(pass.framebuffers[j]->getAttachment(it->second),
									                      name, assetAtatchmentRef->set, assetAtatchmentRef->binding, "");
									effectPipeDef.descSetExists[assetAtatchmentRef->set] = true;
									effectPipeDef.descSetIsMultibuffered[assetAtatchmentRef->set] = true;
								}
							}
						}
					}


					/////// ASSIGN THE SAMPLERS ///////
					auto samplers_it = samplersIndexedByPipeAndTexture.find(pipe_it->pipelineName);

					if (samplers_it != samplersIndexedByPipeAndTexture.end())
					{
						for (auto textures_it = pipedef->textures.begin(); textures_it != pipedef->textures.end(); ++textures_it)
						{
							if (!textures_it->semantic.empty())
							{
								auto tex_sampler_it = samplers_it->second.find(textures_it->semantic);
								if (tex_sampler_it != samplers_it->second.end())
								{
									Sampler sampler = tex_sampler_it->second.sampler;
									effectPipeDef.textureSamplersByTexSemantic[tex_sampler_it->first] = tex_sampler_it->second;
								}
								else
								{
									Log("EffectApi: Could not find a sampler for texture [%s], pipeline [%s] referenced in pass #%d",
									    tex_sampler_it->first.c_str(), pipe_it->pipelineName.c_str(), pass_idx);
								}

							}
							else if (!textures_it->textureName.empty())
							{
								auto tex_sampler_it = samplers_it->second.find(textures_it->textureName);
								if (tex_sampler_it != samplers_it->second.end())
								{
									effectPipeDef.textureSamplersByTexName[tex_sampler_it->first] = tex_sampler_it->second;
								}
								else
								{
									Log("EffectApi: Could not find a sampler for texture [%s], pipeline [%s] referenced in pass #%d",
									    tex_sampler_it->first.c_str(), pipe_it->pipelineName.c_str(), pass_idx);
								}
							}
							else
							{
								Log("EffectApi: Found texture for which neither name nor semantic was defined: pipeline [%s] "
								    "referenced in pass #%d", pipe_it->pipelineName.c_str(), pass_idx);
							}
						}
					}
					else if (!pipedef->textures.empty()) //We have textures, but no samplers
					{
						Log("EffectApi: initialization: Pipeline [%s] referenced in pass #%d had textures, but no samplers were defined for them.",
						    pipe_it->pipelineName.c_str(), pass_idx);
						continue;
					}

					/////// ASSIGN THE PIPELINE LAYOUT ///////
					auto layouts_it = layouts.find(pipe_it->pipelineName);

					if (layouts_it == layouts.end())
					{
						Log("EffectApi initialization: Could not find a layout for pipeline [%s] referenced in pass #%d",
						    pipe_it->pipelineName.c_str(), pass_idx);
						continue;
					}
					else if (layouts_it->second.isNull())
					{
						Log("EffectApi initialization: Layout for pipeline [%s] referenced in pass #%d was null", pipe_it->pipelineName.c_str(), pass_idx);
						continue;
					}
					cp.pipelineLayout = layouts_it->second;

					/////// CONFIGURE BLENDING ETC ///////
					// add blending states for the attachment if the target is a valid std::string and not 'none'
					for (uint32_t i = 0; i < assets::effect::Subpass::MaxTargets; ++i)
					{
						if (subpass_it->targets[i].empty() || subpass_it->targets[i] == "none") { continue; }
						cp.colorBlend.setAttachmentState(i, utils::convertToVk(pipedef->blending));
					}
					cp.renderPass = pass.renderPass;
					cp.subpass = subpass_idx;

					////// CONFIGURE DEPTHSTENCILSTATES //////
					if (!subpass_it->useDepthStencil) { cp.depthStencil.enableAllStates(false); }
					cp.depthStencil.enableDepthWrite(pipedef->enableDepthWrite);
					cp.depthStencil.enableDepthTest(pipedef->enableDepthTest);
					cp.depthStencil.setDepthCompareFunc((VkCompareOp)pipedef->depthCmpFunc);
					cp.depthStencil.enableStencilTest(pipedef->enableStencilTest);
					cp.depthStencil
					.setStencilFront(utils::convertToVk(pipedef->stencilFront))
					.setStencilBack(utils::convertToVk(pipedef->stencilBack));

					///// CONFIGURE RASTER STATES /////
					cp.rasterizer.setCullMode((VkCullModeFlags)pipedef->cullFace);
					cp.rasterizer.setFrontFaceWinding((VkFrontFace)pipedef->windingOrder);

					///// CONFIGURE VERTEXINPUT BINDING /////
					for (uint32_t i = 0; i < pipedef->vertexBinding.size(); ++i)
					{
						cp.vertexInput.addInputBinding(pvrvk::VertexInputBindingDescription(pipedef->vertexBinding[i].index, 0,
						                               (VkVertexInputRate)pipedef->vertexBinding[i].stepRate));
					}

					/////// CONFIGURE SHADERS ETC ///////
					for (auto shader_it = pipedef->shaders.begin(); shader_it != pipedef->shaders.end(); ++shader_it)
					{
						Shader shader = effect.getDevice()->createShader(BufferStream("VertexShader", (*shader_it)->source.data(), (*shader_it)->source.length()).readToEnd<uint32_t>());
						if (shader.isNull())
						{
							Log("EffectApi initialization: Failed to create shader with name [%s]", (*shader_it)->name.c_str());
							continue;
						}
						switch ((*shader_it)->type)
						{
						case ShaderType::VertexShader: cp.vertexShader = shader; break;
						case ShaderType::FragmentShader: cp.fragmentShader = shader; break;
						case ShaderType::GeometryShader: cp.geometryShader = shader; break;
						case ShaderType::TessControlShader: cp.tesselationStates.setControlShader(shader); break;
						case ShaderType::TessEvaluationShader: cp.tesselationStates.setEvaluationShader(shader); break;
						default: Log("EffectApi initialization: Shader with name [%s] had unknown type", (*shader_it)->name.c_str()); break;
						}
					}

					for (auto& assetuniform : pipedef->uniforms)
					{
						auto& apiuniform = effectPipeDef.uniforms[assetuniform.semantic];
						static_cast<assets::effect::UniformSemantic&>(apiuniform) = assetuniform;
					}
				}
			}
		}
	}
}

struct TempDescBinding
{
	DescriptorSetLayoutCreateInfo desclayoutcreateparam;
	bool active;
	TempDescBinding() : active(false) {}
};

struct TempDescBindings
{
	TempDescBinding layouts[4];
	uint16_t pipe_tmp_asset_idx;
};

struct TempPipeIdAndSetNo
{
	uint16_t pipe_id;
	uint16_t set_no;
	TempPipeIdAndSetNo() {}
	TempPipeIdAndSetNo(uint16_t pipe_id, uint16_t set_no) : pipe_id(pipe_id), set_no(set_no) {}
};

struct TempListOfSetsEntry
{
	DescriptorSetLayout desclayout;
	std::vector<TempPipeIdAndSetNo> pipeids_setnos;
};

void createLayouts(Effect_& effect, std::map<StringHash, PipelineLayout>& pipeLayoutsIndexed)
{
	//This function will iterate over the effect in order to detect all pipeline layouts, detect any duplicates,
	//only create layouts for the ones required, and then map each pipeline to one of them.
	DeviceWeakPtr& device = effect.getDevice();

	const assets::effect::Effect& assetEffect = effect.getEffectAsset();

	auto versioned_pipelines_it = assetEffect.versionedPipelines.find(effect.getApiString());

	auto& asset_pipes = versioned_pipelines_it->second;

	std::vector<TempDescBindings> all_sets_and_duplicates;
	all_sets_and_duplicates.reserve(asset_pipes.size() * 4); // Just a starting number... No point in multiple allocations.

	uint16_t pipe_idx = 0;
	for (auto it_pipe = asset_pipes.begin(); it_pipe != asset_pipes.end(); ++it_pipe)
	{
		all_sets_and_duplicates.resize(all_sets_and_duplicates.size() + 1);
		TempDescBindings& pipeBindings = all_sets_and_duplicates.back();
		pipeBindings.pipe_tmp_asset_idx = pipe_idx;
		for (auto it_buff = it_pipe->second.buffers.begin();
		     it_buff != it_pipe->second.buffers.end(); ++it_buff)
		{

			pipeBindings.layouts[it_buff->set].desclayoutcreateparam.setBinding(
			  it_buff->binding, (VkDescriptorType)it_buff->type, 1,
			  VkShaderStageFlags::e_ALL_GRAPHICS);
			pipeBindings.layouts[it_buff->set].active = true;
		}
		for (auto it_tex = it_pipe->second.textures.begin();
		     it_tex != it_pipe->second.textures.end(); ++it_tex)
		{
			pipeBindings.layouts[it_tex->set].desclayoutcreateparam.setBinding(
			  it_tex->binding, VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 1,
			  VkShaderStageFlags::e_ALL_GRAPHICS);
			pipeBindings.layouts[it_tex->set].active = true;
		}
		for (auto it_inputs = it_pipe->second.inputAttachments.begin();
		     it_inputs != it_pipe->second.inputAttachments.end(); ++it_inputs)
		{
			pipeBindings.layouts[it_inputs->set]
			.desclayoutcreateparam.setBinding(
			  it_inputs->binding, VkDescriptorType::e_INPUT_ATTACHMENT, 1,
			  VkShaderStageFlags::e_ALL_GRAPHICS);
			pipeBindings.layouts[it_inputs->set].active = true;
		}
		++pipe_idx;
	}

	//REMOVE ALL DUPLICATES!
	//This data structure will keep a list of each active descriptor set, together with what pipes it belongs to.
	//Afterwards, we can use it to create a "proper" list. For example, have, for each pipe, an index to a simple list...

	//Destroying all duplicates, we can actually also create the real DescriptorSetLayouts.
	std::vector<TempListOfSetsEntry> sets_with_pipe_ids;
	sets_with_pipe_ids.reserve(all_sets_and_duplicates.size() / 2); //assume half duplicate.

	for (size_t outer = 0; outer < all_sets_and_duplicates.size(); ++outer)
	{
		//We will be adding one to the list, then making sure no other duplicates of it exist in the list.
		//So, at each of these lines, each one MUST be unique, or it would have been removed already.
		for (size_t outer_set = 0; outer_set < 4; ++outer_set)
		{
			auto& current_set = all_sets_and_duplicates[outer].layouts[outer_set];
			if (current_set.active)
			{
				// Add it to our list...
				sets_with_pipe_ids.resize(sets_with_pipe_ids.size() + 1);
				sets_with_pipe_ids.back().desclayout = device->createDescriptorSetLayout(current_set.desclayoutcreateparam);
				sets_with_pipe_ids.back().pipeids_setnos.push_back(TempPipeIdAndSetNo(static_cast<uint16_t>(outer), static_cast<uint16_t>(outer_set)));

				// ...and remove any duplicates:
				// ...Traverse the rest of the sets...
				for (size_t inner = outer + 1; inner < all_sets_and_duplicates.size(); ++inner)
				{
					for (size_t inner_set = 0; inner_set < 4; ++inner_set)
					{
						auto& current_inner_set = all_sets_and_duplicates[inner].layouts[inner_set];
						if (current_inner_set.active)
						{
							if (current_set.desclayoutcreateparam == current_inner_set.desclayoutcreateparam)
							{
								current_inner_set.active = false;
								current_inner_set.desclayoutcreateparam.clear();
								sets_with_pipe_ids.back().pipeids_setnos.push_back(TempPipeIdAndSetNo(static_cast<uint16_t>(inner), static_cast<uint16_t>(inner_set)));
							}
						}
					}
				}
			}
		}
	}

	//Now, use these to add a pipeline -> pipeline layout effect
	//Can we additionally spot "compatible" pipeline layouts?
	std::vector<PipelineLayoutCreateInfo> pipeLayoutCps;
	pipeLayoutCps.resize(asset_pipes.size());
	for (auto it = sets_with_pipe_ids.begin(); it != sets_with_pipe_ids.end(); ++it)
	{
		for (auto it2 = it->pipeids_setnos.begin(); it2 != it->pipeids_setnos.end(); ++it2)
		{
			pipeLayoutCps[it2->pipe_id].setDescSetLayout(it2->set_no, it->desclayout);
		}
	}

	//The actual pipeline layouts! Shared refcounting makes sure of no duplication.
	std::vector<PipelineLayout> pipeLayouts;
	pipeLayouts.resize(pipeLayoutCps.size());

	//Inner loop: We traverse the list twice
	for (size_t outer = 0; outer < pipeLayoutCps.size(); ++outer)
	{
		if (pipeLayouts[outer].isNull())
		{
			// Create a new layout if one does not exist, else, do nothing.
			pipeLayouts[outer] = device->createPipelineLayout(pipeLayoutCps[outer]);

			// Traverse the rest of the list, and assign the same pipe layout to the rest of the pipes.
			for (size_t inner = outer + 1; inner != pipeLayoutCps.size(); ++inner)
			{
				if (pipeLayoutCps[outer] == pipeLayoutCps[inner])
				{
					pipeLayouts[inner] = pipeLayouts[outer];
				}
			}
		}
	}

	size_t idx = 0;
	for (auto it = asset_pipes.begin(); it != asset_pipes.end(); ++it)
	{
		pipeLayoutsIndexed[it->first] = pipeLayouts[idx++];
	}
}

struct TempSamplers
{
	std::vector<PackedSamplerFilter> samplerPerTextureInOrder;
	uint16_t pipe_tmp_asset_idx;
};

struct TempPipeIdAndTextureNo
{
	uint16_t pipe_id;
	uint16_t tex_no;
	TempPipeIdAndTextureNo() {}
	TempPipeIdAndTextureNo(uint16_t pipe_id, uint16_t tex_id) : pipe_id(pipe_id), tex_no(tex_id) {}
};

struct TempListOfSamplersEntry
{
	Sampler sampler;
	std::vector<TempPipeIdAndTextureNo> pipeids_texturenos;
};

void createSamplers(Effect_& effect, std::map<StringHash,
                    std::map<StringHash, effect::TextureInfo>/**/>& textureInfoByPipeAndTex)
{
	DeviceWeakPtr device = effect.getDevice();

	const assets::effect::Effect& assetEffect = effect.getEffectAsset();

	auto versioned_pipelines_it = assetEffect.versionedPipelines.find(effect.getApiString());

	auto& asset_pipes = versioned_pipelines_it->second;

	std::vector<TempSamplers> all_samplers_with_duplicates;
	all_samplers_with_duplicates.reserve(asset_pipes.size() * 2); // Just a starting number... No point in multiple allocations.

	Sampler defaultSampler = effect.getDevice()->createSampler(pvrvk::SamplerCreateInfo());

	uint16_t pipe_idx = 0;
	for (auto it_pipe = asset_pipes.begin(); it_pipe != asset_pipes.end(); ++it_pipe)
	{
		all_samplers_with_duplicates.resize(all_samplers_with_duplicates.size() + 1);
		all_samplers_with_duplicates.back().pipe_tmp_asset_idx = pipe_idx;
		for (auto it_tex = it_pipe->second.textures.begin(); it_tex != it_pipe->second.textures.end(); ++it_tex)
		{
			all_samplers_with_duplicates.back().samplerPerTextureInOrder.push_back(it_tex->samplerFilter);
		}
		++pipe_idx;
	}

	//REMOVE ALL DUPLICATES!
	//This data structure will keep a list of each active sampler set, together with what pipes it belongs to.
	//Afterwards, we can use it to create the "proper" list.

	//Destroying all duplicates, we can actually also create the real DescriptorSetLayouts.
	std::vector<TempListOfSamplersEntry> samplers_with_pipe_ids;
	samplers_with_pipe_ids.reserve(20); //shouldn't be that many...

	for (size_t outer = 0; outer < all_samplers_with_duplicates.size(); ++outer)
	{
		//We will be adding one to the list, then making sure no other duplicates of it exist in the list.
		//So, at each of these lines, each one MUST be unique, or it would have been removed already.
		for (size_t outer_tex = 0; outer_tex < all_samplers_with_duplicates[outer].samplerPerTextureInOrder.size(); ++outer_tex)
		{
			auto& current_sampler = all_samplers_with_duplicates[outer].samplerPerTextureInOrder[outer_tex];
			if (current_sampler != PackedSamplerFilter(-1))
			{
				//NEW ONE. Create a sampler for it.
				SamplerCreateInfo param;

				utils::unpackSamplerFilter(current_sampler,  param.minFilter, param.magFilter, param.mipMapMode);

				// Add it to our list...
				samplers_with_pipe_ids.resize(samplers_with_pipe_ids.size() + 1);
				Sampler sampler = device->createSampler(param);
				samplers_with_pipe_ids.back().sampler = sampler;
				samplers_with_pipe_ids.back().pipeids_texturenos.push_back(
				  TempPipeIdAndTextureNo(static_cast<uint16_t>(outer), static_cast<uint16_t>(outer_tex)));

				// ...and remove any duplicates:
				// ...Traverse the rest of the list...
				for (size_t inner = outer + 1; inner < all_samplers_with_duplicates.size(); ++inner)
				{
					for (size_t inner_tex = 0; inner_tex < all_samplers_with_duplicates[inner]
					     .samplerPerTextureInOrder.size(); ++inner_tex)
					{
						auto& current_inner_sampler =
						  all_samplers_with_duplicates[inner].samplerPerTextureInOrder[inner_tex];
						//If it is active...
						if (current_inner_sampler != PackedSamplerFilter(-1))
						{
							//If it is the same as the current one...
							if (current_sampler == current_inner_sampler)
							{
								//Add a reference to the original list - which item this one will be referring to.
								current_inner_sampler = PackedSamplerFilter(-1); //Deactivate it
								//And add a reference to it in the sampler's list.
								samplers_with_pipe_ids.back().pipeids_texturenos.push_back(
								  TempPipeIdAndTextureNo(static_cast<uint16_t>(inner), static_cast<uint16_t>(inner_tex)));
							}
						}
					}
				}
			}
		}
	}

	//Now, we have a list of samplers, along with the items that each belongs to!
	//Now, we create a flat list of samplers - the list can be flat only because we are DIRECTLY ITERATING
	std::vector<std::vector<Sampler>/**/> samplers;
	samplers.resize(asset_pipes.size());
	for (auto it = samplers_with_pipe_ids.begin(); it != samplers_with_pipe_ids.end(); ++it)
	{
		for (auto it2 = it->pipeids_texturenos.begin(); it2 != it->pipeids_texturenos.end(); ++it2)
		{
			if (samplers[it2->pipe_id].size() <= it2->tex_no)
			{
				samplers[it2->pipe_id].resize(it2->tex_no + 1);
			}
			samplers[it2->pipe_id][it2->tex_no] = it->sampler;
		}
	}

	size_t idx1 = 0;
	for (auto it = asset_pipes.begin(); it != asset_pipes.end(); ++it)
	{
		size_t idx2 = 0;
		for (auto it2 = it->second.textures.begin(); it2 != it->second.textures.end(); ++it2)
		{
			if (!it2->textureName.empty())
			{
				auto& tmp = textureInfoByPipeAndTex[it->first][it2->textureName];
				static_cast<assets::effect::TextureRef&>(tmp) = static_cast<const assets::effect::TextureRef&>(*it2);
				tmp.sampler = samplers[idx1][idx2];
			}
			if (!it2->semantic.empty())
			{
				auto& tmp = textureInfoByPipeAndTex[it->first][it2->semantic];
				tmp.sampler = samplers[idx1][idx2];
				static_cast<assets::effect::TextureRef&>(tmp) = static_cast<const assets::effect::TextureRef&>(*it2);
			}
			++idx2;
		}
		++idx1;
	}
}


bool createFixedDescriptorSets(Effect_& effect,
                               std::map<StringHash, PipelineDef>& pipelines, std::map<StringHash,
                               PipelineLayout>& pipelineLayouts, uint32_t swapchainLength)
{
	std::map<DescriptorSetLayout, Multi<DescriptorSet>/**/> sets;
	for (auto& pipeDef : pipelines)
	{
		auto layout_it = pipelineLayouts.find(pipeDef.first);
		assertion(layout_it != pipelineLayouts.end(), strings::createFormatted(
		            "Effectvk::init Pipeline layout was not created correctly for pipeline [%s]",
		            pipeDef.first.c_str()));

		auto& pipelayout = layout_it->second;

		assertion(pipelayout.isValid(), strings::createFormatted(
		            "Effectvk::init Pipeline layout was not created correctly for pipeline[%s]",
		            pipeDef.first.c_str()));

		uint32_t count = pipelayout->getNumDescriptorSetLayouts();
		for (uint32_t i = 0; i < count; ++i)
		{
			auto& setlayout = pipelayout->getDescriptorSetLayout(i);
			assertion(setlayout.isValid(), strings::createFormatted(
			            "Effectvk::init Descriptor set layout [%d] for pipeline[%s] was \"Fixed\", "
			            "but it had not been created", i, pipeDef.first.c_str()));

			if (pipeDef.second.descSetIsFixed[i])
			{
				auto& set = sets[setlayout];
				uint32_t numsets = pipeDef.second.descSetIsMultibuffered[i] ? swapchainLength : 1;
				for (uint32_t swapindex = 0; swapindex < numsets; ++swapindex)
				{
					if (set[swapindex].isNull())
					{
						set[swapindex] = effect.getDescriptorPool()->allocateDescriptorSet(setlayout);
						if (!set[swapindex].isValid())
						{
							Log("EffectApi: Failed to create pipeline %s descriptor set for swapchain %d",
							    pipeDef.first.str().c_str(), swapindex);
							return false;
						}
					}
				}
				pipeDef.second.fixedDescSet[i] = set;
			}
		}
	}
	return true;
}
}//

void Effect_::registerUniformSemantic(StringHash pipeline,
                                      StringHash semantic, StringHash variableName)
{
	PipelineDef* pipe = getPipelineDefinition(pipeline);
	if (pipe) { pipe->uniforms[semantic] = UniformSemantic(semantic, variableName);}
}

void Effect_::registerTextureSemantic(StringHash pipeline,
                                      StringHash semantic, uint16_t set, uint16_t binding)
{
	PipelineDef* pipe = getPipelineDefinition(pipeline);
	if (pipe) { pipe->textures[semantic] = ObjectSemantic(semantic, set, binding);}
}

bool Effect_::buildRenderObjects(CommandBuffer& texUploadCmdBuffer,
                                 IAssetProvider& assetProvider, std::vector<utils::ImageUploadResults>& uploadResults)
{
	std::map<StringHash, PipelineLayout> pipeLayoutsIndexed;
	std::map<StringHash, std::map<StringHash, TextureInfo>/**/> samplersIndexedByPipeAndTexture;
	createLayouts(*this, pipeLayoutsIndexed);
	createSamplers(*this, samplersIndexedByPipeAndTexture);
	createPasses(*this, _passes, pipeLayoutsIndexed, _pipelineDefinitions,
	             samplersIndexedByPipeAndTexture, _swapchain->getSwapchainLength());
	createTextures(*this, _textures, texUploadCmdBuffer, assetProvider, uploadResults);
	createBuffers(*this, _pipelineDefinitions, _bufferDefinitions,
	              _swapchain->getSwapchainLength());

	_descriptorPool = _device->createDescriptorPool(DescriptorPoolCreateInfo()
	                  .addDescriptorInfo(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 32)
	                  .addDescriptorInfo(VkDescriptorType::e_UNIFORM_BUFFER, 16)
	                  .addDescriptorInfo(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 16)
	                  .addDescriptorInfo(VkDescriptorType::e_STORAGE_BUFFER, 16)
	                  .addDescriptorInfo(VkDescriptorType::e_STORAGE_BUFFER_DYNAMIC, 16)
	                  .addDescriptorInfo(VkDescriptorType::e_INPUT_ATTACHMENT, 16));

	return createFixedDescriptorSets(*this, _pipelineDefinitions,
	                                 pipeLayoutsIndexed, _swapchain->getSwapchainLength());
}

Effect_::Effect_(const DeviceWeakPtr& device) : _device(device)
{
}


bool Effect_::init(const assets::effect::Effect& effect, Swapchain& swapchain,
                   CommandBuffer& cmdBuffer, IAssetProvider& assetProvider,
                   std::vector<utils::ImageUploadResults>& uploadResults)
{
	// bypass the warning
	static bool firsttime = initializeStringLists(); (void)firsttime;
	_swapchain = swapchain;
	_assetEffect = effect;

	_apiString = findMatchingApiString(_assetEffect, Api::Vulkan);

	_name = effect.name;
	return buildRenderObjects(cmdBuffer, assetProvider, uploadResults);
}
}
}
}
//!\endcond
