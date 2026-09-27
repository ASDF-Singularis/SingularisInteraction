/* ====================================================================== *
 * SingularisInteractionViewInterface.h                                   *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2026 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2026 TrifingZW. All Rights Reserved.                     *
 * Created: 2026/09/27 | Author: TrifingZW                                *
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

#pragma once

#include <CoreMinimal.h>
#include <UObject/Interface.h>

#include "SingularisInteractionViewInterface.generated.h"

/**
 * 引力奇点交互视图接口。
 *
 * 交互视图契约：实现者接收交互组件的全量刷新与增量事件，由交互控件组件经 Execute_ 调用。
 * 实现者不限于控件——任意 UObject 实现本接口即可接入驱动。
 */
UINTERFACE(Blueprintable, BlueprintType)
class USingularisInteractionViewInterface : public UInterface
{
	GENERATED_BODY()
};

class SINGULARISINTERACTION_API ISingularisInteractionViewInterface
{
	GENERATED_BODY()

public:
	/**
	 * 交互状态全量刷新：启用与悬浮。
	 *
	 * 由交互控件组件在绑定完成后主动调用，消除错过事件导致的空白期。
	 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互视图接口",
		meta = (DisplayName = "交互刷新")
	)
	void OnRefresh(bool bEnabled, bool bHovered);

	/** 交互触发。 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互视图接口",
		meta = (DisplayName = "触发")
	)
	void OnTrigger();

	/** 交互悬浮。 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互视图接口",
		meta = (DisplayName = "悬浮")
	)
	void OnHover();

	/** 交互未悬浮。 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互视图接口",
		meta = (DisplayName = "未悬浮")
	)
	void OnUnhover();

	/** 进入交互提示范围。 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互视图接口",
		meta = (DisplayName = "进入范围")
	)
	void OnEnterRange();

	/** 离开交互提示范围。 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互视图接口",
		meta = (DisplayName = "离开范围")
	)
	void OnExitRange();
};
