/*!
\brief Function definitions for the Queue class
\file PVRVk/QueueVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "PVRVk/QueueVk.h"
#include "PVRVk/SwapchainVk.h"
#include "PVRVk/CommandBufferVk.h"

namespace pvrvk {
namespace impl {
VkResult Queue_::submit(const SubmitInfo* queueSubmitInfo, uint32_t numSubmitInfos, Fence signalFence)
{
	uint32_t numTotalSemaphores = 0;
	uint32_t numTotalCommandBuffers = 0;
	std::vector<VkSubmitInfo> vkSubmitInfos(numSubmitInfos, VkSubmitInfo{});
	for (uint32_t i = 0; i < numSubmitInfos; ++i)
	{
		// find the total number of command buffers and semaphores
		numTotalCommandBuffers += queueSubmitInfo[i].numCommandBuffers;
		numTotalSemaphores += queueSubmitInfo[i].numWaitSemaphores + queueSubmitInfo[i].numSignalSemaphores;
	}

	std::vector<VkSemaphore> semaphoresVk(numTotalSemaphores);
	std::vector<VkCommandBuffer> commandBuffersVk(numTotalCommandBuffers);

	uint32_t currentSemaphoreIndex = 0;
	uint32_t currentCommandbufferIndex = 0;

	for (uint32_t i = 0; i < numSubmitInfos; ++i)
	{
		const SubmitInfo& submitInfo = queueSubmitInfo[i];
		vkSubmitInfos[i].sType = VkStructureType::e_SUBMIT_INFO;
		vkSubmitInfos[i].pWaitDstStageMask = (VkPipelineStageFlags*)submitInfo.waitDestStages;

		vkSubmitInfos[i].commandBufferCount = submitInfo.numCommandBuffers;
		if (submitInfo.numCommandBuffers > 0)
		{
			vkSubmitInfos[i].pCommandBuffers = &commandBuffersVk[currentCommandbufferIndex];

			for (uint32_t j = 0; j < submitInfo.numCommandBuffers; ++j)
			{
				commandBuffersVk[currentCommandbufferIndex + j] = submitInfo.commandBuffers[j]->getNativeObject();
			}
		}
		else
		{
			vkSubmitInfos[i].pCommandBuffers = nullptr;
		}

		// increment the current command buffer index to use for the next submitInfo
		currentCommandbufferIndex += submitInfo.numCommandBuffers;

		vkSubmitInfos[i].signalSemaphoreCount = submitInfo.numSignalSemaphores;
		if (submitInfo.numSignalSemaphores > 0)
		{
			vkSubmitInfos[i].pSignalSemaphores = &semaphoresVk[currentSemaphoreIndex];

			for (uint32_t j = 0; j < submitInfo.numSignalSemaphores; ++j)
			{
				semaphoresVk[currentSemaphoreIndex + j] = submitInfo.signalSemaphores[j]->getNativeObject();
			}
		}
		else
		{
			vkSubmitInfos[i].pSignalSemaphores = nullptr;
		}
		// increment the current semaphore index to use for the wait semaphores
		currentSemaphoreIndex += submitInfo.numSignalSemaphores;

		vkSubmitInfos[i].waitSemaphoreCount = submitInfo.numWaitSemaphores;
		if (submitInfo.numWaitSemaphores > 0)
		{
			vkSubmitInfos[i].pWaitSemaphores = &semaphoresVk[currentSemaphoreIndex];

			for (uint32_t j = 0; j < submitInfo.numWaitSemaphores; ++j)
			{
				semaphoresVk[currentSemaphoreIndex + j] = submitInfo.waitSemaphores[j]->getNativeObject();
			}
		}
		else
		{
			vkSubmitInfos[i].pWaitSemaphores = nullptr;
		}
		// increment the current semaphore index to use for the next submitInfo
		currentSemaphoreIndex += submitInfo.numWaitSemaphores;
	}

	VkResult result = vk::QueueSubmit(_vkQueue, static_cast<uint32_t>(vkSubmitInfos.size()), vkSubmitInfos.data(),
	                                  (signalFence.isValid() ? signalFence->getNativeObject() : VK_NULL_HANDLE));

	if (result != VkResult::e_SUCCESS)
	{
		Log("CommandBufferBase::submitCommandBuffers failed");
	}
	return (VkResult)result;
}

VkResult Queue_::present(PresentInfo& presentInfo)
{
	std::vector<VkSwapchainKHR> swapchainsVector(presentInfo.numSwapchains);
	std::vector<uint32_t> imageIndices(presentInfo.numSwapchains);

	for (uint32_t i = 0; i < presentInfo.numSwapchains; ++i)
	{
		swapchainsVector[i] = presentInfo.swapchains[i]->getNativeObject();
		imageIndices[i] = presentInfo.imageIndices[i];
	}

	std::vector<VkSemaphore> waitSemaphores(presentInfo.numWaitSemaphores);
	for (uint32_t i = 0; i < presentInfo.numWaitSemaphores; ++i)
	{
		waitSemaphores[i] = presentInfo.waitSemaphores[i]->getNativeObject();
	}

	VkResult result = VkResult::e_SUCCESS;
	VkPresentInfoKHR presentInfoVk = {};
	presentInfoVk.sType = VkStructureType::e_PRESENT_INFO_KHR;
	presentInfoVk.swapchainCount = presentInfo.numSwapchains;
	presentInfoVk.pSwapchains = swapchainsVector.data();
	presentInfoVk.pImageIndices = &imageIndices[0];
	presentInfoVk.pWaitSemaphores = waitSemaphores.data();
	presentInfoVk.waitSemaphoreCount = presentInfo.numWaitSemaphores;
	presentInfoVk.pResults = &result;

	if (!vkIsSuccessful(vk::QueuePresentKHR(_vkQueue, &presentInfoVk), "Error in queue present"))
	{
		return (VkResult)result;
	}
	if (result != VkResult::e_SUCCESS)
	{
		Log("Queue present was unsuccessful");
	}
	return (VkResult)result;
}

VkResult Queue_::waitIdle()
{
	VkResult rslt = vk::QueueWaitIdle(_vkQueue);
	vkIsSuccessful(rslt, "Queue::waitIdle - error in preceeding command.");
	return (VkResult)rslt;
}
namespace {
inline void processSparseMemoryBind(
  const SparseMemoryBind& memoryBind,
  VkSparseMemoryBind& outVkSparseMemoryBind)
{
	outVkSparseMemoryBind.memory = memoryBind.memory->getNativeObject();
	outVkSparseMemoryBind.flags = (VkSparseMemoryBindFlags)memoryBind.flags;
	outVkSparseMemoryBind.memoryOffset = memoryBind.memoryOffset;
	outVkSparseMemoryBind.resourceOffset = memoryBind.resourceOffset;
	outVkSparseMemoryBind.size = memoryBind.size;
}

inline void processSparseBufferMemoryBindInfo(
  const SparseBufferMemoryBindInfo& sparseBufferMemBindInfo,
  std::vector<VkSparseMemoryBind>& inOutVkSparseMemBinding,
  VkSparseBufferMemoryBindInfo& outSparseBufferMemoryBindInfo)
{
	uint32_t offset  = static_cast<uint32_t>(inOutVkSparseMemBinding.size());
	outSparseBufferMemoryBindInfo.buffer = sparseBufferMemBindInfo.buffer->getNativeObject();
	outSparseBufferMemoryBindInfo.bindCount = static_cast<uint32_t>(sparseBufferMemBindInfo.binds.size());
	inOutVkSparseMemBinding.resize(inOutVkSparseMemBinding.size() + sparseBufferMemBindInfo.binds.size());
	outSparseBufferMemoryBindInfo.pBinds = &inOutVkSparseMemBinding[offset];

	// do the sparse memory bindings
	std::for_each(sparseBufferMemBindInfo.binds.begin(), sparseBufferMemBindInfo.binds.end(),
	              [&](const SparseMemoryBind & sparseMemBind)
	{
		processSparseMemoryBind(sparseMemBind, inOutVkSparseMemBinding[offset++]);
	});
}

inline void processSparseImageOpaqueMemoryBindInfo(
  const SparseImageOpaqueMemoryBindInfo& sparseImageOpaqueMemoryBindInfo,
  std::vector<VkSparseMemoryBind>& inOutVkSparseMemBinding,
  VkSparseImageOpaqueMemoryBindInfo& outSparseImageOpaqueMemoryBindInfo)
{
	uint32_t offset  = static_cast<uint32_t>(inOutVkSparseMemBinding.size());
	outSparseImageOpaqueMemoryBindInfo.image  = sparseImageOpaqueMemoryBindInfo.image->getNativeObject();

	outSparseImageOpaqueMemoryBindInfo.bindCount = static_cast<uint32_t>(sparseImageOpaqueMemoryBindInfo.binds.size());

	inOutVkSparseMemBinding.resize(inOutVkSparseMemBinding.size() +
	                               sparseImageOpaqueMemoryBindInfo.binds.size());

	outSparseImageOpaqueMemoryBindInfo.pBinds = &inOutVkSparseMemBinding[offset];

	std::for_each(sparseImageOpaqueMemoryBindInfo.binds.begin(),
	              sparseImageOpaqueMemoryBindInfo.binds.end(),
	              [&](const SparseMemoryBind & sparseMemBind)
	{
		processSparseMemoryBind(sparseMemBind, inOutVkSparseMemBinding[offset++]);
	});
}

inline void processSparseImageMemoryBind(
  const SparseImageMemoryBind& sparseImageMemoryBind,
  VkSparseImageMemoryBind& outSparseImageMemoryBind)
{
	outSparseImageMemoryBind.subresource = impl::convertToVk(sparseImageMemoryBind.subresource);

	outSparseImageMemoryBind.offset = VkOffset3D
	{
		sparseImageMemoryBind.offset.x,
		sparseImageMemoryBind.offset.y,
		sparseImageMemoryBind.offset.z,
	};

	outSparseImageMemoryBind.extent = VkExtent3D
	{
		sparseImageMemoryBind.extent.width,
		sparseImageMemoryBind.extent.height,
		sparseImageMemoryBind.extent.depth
	};

	outSparseImageMemoryBind.memory = sparseImageMemoryBind.memory->getNativeObject();
	outSparseImageMemoryBind.offset = VkOffset3D
	{
		sparseImageMemoryBind.offset.x,
		sparseImageMemoryBind.offset.y,
		sparseImageMemoryBind.offset.z,
	};
	outSparseImageMemoryBind.flags = (VkSparseMemoryBindFlags)sparseImageMemoryBind.flags;
}

inline void processSparseImageMemoryBindInfo(
  const SparseImageMemoryBindInfo& sparseImageMemoryBind,
  std::vector<VkSparseImageMemoryBind>& inOutVkSparseImageMemoryBind,
  VkSparseImageMemoryBindInfo& outVkSparseImageMemoryBindInfo)
{
	uint32_t offset = static_cast<uint32_t>(inOutVkSparseImageMemoryBind.size());
	outVkSparseImageMemoryBindInfo.bindCount = static_cast<uint32_t>(sparseImageMemoryBind.binds.size());
	outVkSparseImageMemoryBindInfo.image = sparseImageMemoryBind.image->getNativeObject();

	inOutVkSparseImageMemoryBind.resize(inOutVkSparseImageMemoryBind.size() +
	                                    sparseImageMemoryBind.binds.size());

	outVkSparseImageMemoryBindInfo.pBinds = &inOutVkSparseImageMemoryBind[offset];

	std::for_each(sparseImageMemoryBind.binds.begin(), sparseImageMemoryBind.binds.end(),
	              [&](const SparseImageMemoryBind & sparseImageBindInfo)
	{
		processSparseImageMemoryBind(sparseImageBindInfo, inOutVkSparseImageMemoryBind[offset++]);
	});
}


void processBindSparseInfo(
  const BindSparseInfo& bindSparseInfo,
  std::vector<VkSparseBufferMemoryBindInfo>& inOutVkSparseBufferMemoryBindInfo,
  std::vector<VkSparseImageMemoryBindInfo>& inOutVkSparseImageMemoryBindInfo,
  std::vector<VkSparseImageOpaqueMemoryBindInfo> inOutVkSparseImageOpaqueMemoryBindInfo,
  std::vector<VkSparseImageMemoryBind>& inOutVkSparseImageMemoryBind,
  std::vector<VkSparseMemoryBind>& inOutVkSparseMemoryBind,
  std::vector<VkSemaphore>& inOutSemaphores,
  VkBindSparseInfo& outVkBindSparseInfo)
{
	memset(&outVkBindSparseInfo, 0, sizeof(outVkBindSparseInfo));
	outVkBindSparseInfo.sType = VkStructureType::e_BIND_SPARSE_INFO;
	outVkBindSparseInfo.bufferBindCount = static_cast<uint32_t>(bindSparseInfo.bufferBinds.size());
	outVkBindSparseInfo.imageBindCount = static_cast<uint32_t>(bindSparseInfo.imageBinds.size());
	outVkBindSparseInfo.imageOpaqueBindCount = static_cast<uint32_t>(bindSparseInfo.imageOpaqueBinds.size());

	//--------------------
	// Process the BufferMemoryBindInfo
	uint32_t offset = static_cast<uint32_t>(inOutVkSparseBufferMemoryBindInfo.size());
	inOutVkSparseBufferMemoryBindInfo.resize(bindSparseInfo.bufferBinds.size() +
	    inOutVkSparseBufferMemoryBindInfo.size());
	outVkBindSparseInfo.pBufferBinds = &inOutVkSparseBufferMemoryBindInfo[offset];
	std::for_each(bindSparseInfo.bufferBinds.begin(), bindSparseInfo.bufferBinds.end(),
	              [&](const SparseBufferMemoryBindInfo & sparseBufferMemoryBindInfo)
	{
		processSparseBufferMemoryBindInfo(sparseBufferMemoryBindInfo, inOutVkSparseMemoryBind,
		                                  inOutVkSparseBufferMemoryBindInfo[offset++]);
	});

	//--------------------
	// Process the ImageMemoryBindInfo
	offset = static_cast<uint32_t>(inOutVkSparseImageMemoryBindInfo.size());
	inOutVkSparseImageMemoryBindInfo.resize(inOutVkSparseImageMemoryBindInfo.size() +
	                                        bindSparseInfo.imageBinds.size());
	outVkBindSparseInfo.pImageBinds = &inOutVkSparseImageMemoryBindInfo[offset];
	std::for_each(bindSparseInfo.imageBinds.begin(), bindSparseInfo.imageBinds.end(),
	              [&](const SparseImageMemoryBindInfo & sparseImageMemoryBindInfo)
	{
		processSparseImageMemoryBindInfo(sparseImageMemoryBindInfo, inOutVkSparseImageMemoryBind,
		                                 inOutVkSparseImageMemoryBindInfo[offset++]);
	});

	//--------------------
	// Process the ImageOpaqueueMemoryBindInfo
	offset = static_cast<uint32_t>(inOutVkSparseImageOpaqueMemoryBindInfo.size());
	inOutVkSparseImageOpaqueMemoryBindInfo.resize(inOutVkSparseImageOpaqueMemoryBindInfo.size() +
	    bindSparseInfo.imageOpaqueBinds.size());
	outVkBindSparseInfo.pImageOpaqueBinds = &inOutVkSparseImageOpaqueMemoryBindInfo[offset];
	std::for_each(bindSparseInfo.imageOpaqueBinds.begin(), bindSparseInfo.imageOpaqueBinds.end(),
	              [&](const SparseImageOpaqueMemoryBindInfo & sparseImageOpaqueMemoryBindInfo)
	{
		processSparseImageOpaqueMemoryBindInfo(sparseImageOpaqueMemoryBindInfo,
		                                       inOutVkSparseMemoryBind, inOutVkSparseImageOpaqueMemoryBindInfo[offset++]);
	});

	//--------------------
	// process the wait Semaphores
	offset = static_cast<uint32_t>(inOutSemaphores.size());
	inOutSemaphores.resize(inOutSemaphores.size() + bindSparseInfo.waitSemaphores.size() +
	                       bindSparseInfo.signalSemaphore.size());
	outVkBindSparseInfo.pWaitSemaphores = &inOutSemaphores[offset];
	outVkBindSparseInfo.waitSemaphoreCount = static_cast<uint32_t>(bindSparseInfo.waitSemaphores.size());
	std::for_each(bindSparseInfo.waitSemaphores.begin(), bindSparseInfo.waitSemaphores.end(),
	              [&](const Semaphore & semaphore)
	{
		inOutSemaphores[offset++] = semaphore->getNativeObject();
	});

	//--------------------
	// process the signal Semaphores
	outVkBindSparseInfo.pSignalSemaphores = &inOutSemaphores[offset];
	outVkBindSparseInfo.signalSemaphoreCount = static_cast<uint32_t>(bindSparseInfo.signalSemaphore.size());
	std::for_each(bindSparseInfo.signalSemaphore.begin(), bindSparseInfo.signalSemaphore.end(),
	              [&](const Semaphore & semaphore)
	{
		inOutSemaphores[offset++] = semaphore->getNativeObject();
	});
}
}
VkResult Queue_::bindSparse(const BindSparseInfo* bindInfo, uint32_t numBindInfos, Fence& fenceSignal)
{
	std::vector<VkSemaphore> semaphores;
	std::vector<VkBindSparseInfo> vkBindSparseInfo(numBindInfos);
	std::vector<VkSparseMemoryBind> vkSparseMemoryBind;
	std::vector<VkSparseImageMemoryBind> vkSparseImageMemoryBind;
	std::vector<VkSparseBufferMemoryBindInfo> vkSparseBufferMemBindInfo;
	std::vector<VkSparseImageOpaqueMemoryBindInfo> vkSparseImageOpaqueMemBindInfo;
	std::vector<VkSparseImageMemoryBindInfo> vkSparseImageMemBindInfo;


	for (uint32_t i = 0; i < numBindInfos; ++i) // for each sparse info
	{
		processBindSparseInfo(bindInfo[i], vkSparseBufferMemBindInfo,
		                      vkSparseImageMemBindInfo, vkSparseImageOpaqueMemBindInfo,
		                      vkSparseImageMemoryBind, vkSparseMemoryBind,
		                      semaphores, vkBindSparseInfo[i]);
	}
	return vk::QueueBindSparse(getNativeObject(), numBindInfos, vkBindSparseInfo.data(),
	                           (fenceSignal.isValid() ? fenceSignal->getNativeObject() : VK_NULL_HANDLE));
}
}
}// namespace pvrvk
