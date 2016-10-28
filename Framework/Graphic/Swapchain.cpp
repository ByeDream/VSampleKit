#include "stdafx.h"

#include "Swapchain.h"
#include "GraphicDevice.h"
#include "OutputDevice.h"

using namespace sce;

Framework::Swapchain::Swapchain(GraphicDevice *device)
	: mDevice(device)
{

}

Framework::Swapchain::~Swapchain()
{

}

void Framework::Swapchain::init()
{
	const OutputDevice *_output = mDevice->getOutput();
	const Rect &_wndRect = _output->getWindowRect();
	const U32 _targetWidth = _wndRect.right - _wndRect.left;
	const U32 _targetHeight = _wndRect.bottom - _wndRect.top;
}

void Framework::Swapchain::deinit()
{

}
