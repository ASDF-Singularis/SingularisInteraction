/* ====================================================================== *
 * SingularisInteractionEditorStyle.cpp                                   *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2026 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2026 TrifingZW. All Rights Reserved.                     *
 * Created: 2026/10/15 | Author: TrifingZW                                *
 * Licensed under MIT License                                             *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. *
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   *
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   *
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      *
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 *
 * ====================================================================== */

#include "Styles/SingularisInteractionEditorStyle.h"

#include <Brushes/SlateImageBrush.h>
#include <Interfaces/IPluginManager.h>
#include <Misc/Paths.h>
#include <Styling/SlateStyle.h>
#include <Styling/SlateStyleRegistry.h>

namespace
{
	/** 样式集实例：模块生命周期内持有，Register / Unregister 成对维护 */
	TSharedPtr<FSlateStyleSet> StyleSetInstance = nullptr;

	/** 节点图标显示尺寸（蓝图图表与调色板通用） */
	const FVector2D IconSize(32.0f, 32.0f);

	/** 插件 Resources 目录下图标资源的绝对路径（插件缺失时返回空串） */
	FString GetIconPath()
	{
		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("SingularisInteraction"));
		if (!Plugin.IsValid())
			return FString();

		return FPaths::Combine(Plugin->GetBaseDir(), TEXT("Resources"), TEXT("Event.png"));
	}
}

FName FSingularisInteractionEditorStyle::GetStyleSetName()
{
	static const FName StyleSetName(TEXT("SingularisInteractionEditor"));
	return StyleSetName;
}

FName FSingularisInteractionEditorStyle::GetSessionEventIconName()
{
	static const FName IconName(TEXT("SingularisInteraction.SessionEvent"));
	return IconName;
}

void FSingularisInteractionEditorStyle::Register()
{
	if (StyleSetInstance.IsValid())
		return;

	StyleSetInstance = MakeShared<FSlateStyleSet>(GetStyleSetName());

	const FString IconPath = GetIconPath();
	if (!IconPath.IsEmpty())
	{
		StyleSetInstance->Set(
			GetSessionEventIconName(),
			new FSlateImageBrush(IconPath, IconSize)
		);
	}

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSetInstance);
}

void FSingularisInteractionEditorStyle::Unregister()
{
	if (!StyleSetInstance.IsValid())
		return;

	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSetInstance);
	StyleSetInstance.Reset();
}
