/* ====================================================================== *
 * SingularisInteractorViewInterface.h                                    *
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
#include <GameplayTagContainer.h>
#include <UObject/Interface.h>

#include "SingularisInteractorViewInterface.generated.h"

class USingularisInteractionComponent;

/**
 * 引力奇点交互者视图接口。
 *
 * 交互者视图契约：实现者接收交互者组件的全量刷新与增量事件，由交互者控件组件经 Execute_ 调用。
 * 实现者不限于控件——任意 UObject 实现本接口即可接入驱动。
 */
UINTERFACE(Blueprintable, BlueprintType)
class USingularisInteractorViewInterface : public UInterface
{
	GENERATED_BODY()
};

class SINGULARISINTERACTION_API ISingularisInteractorViewInterface
{
	GENERATED_BODY()

public:
	/**
	 * 交互者状态全量刷新：当前锁定的交互目标。
	 *
	 * 由交互者控件组件在绑定完成后主动调用，消除错过事件导致的空白期。
	 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互者视图接口",
		meta = (DisplayName = "交互者刷新")
	)
	void OnRefresh(USingularisInteractionComponent* Target);

	/** 交互者锁定的目标变更。 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互者视图接口",
		meta = (DisplayName = "目标变更")
	)
	void OnTargetChanged(USingularisInteractionComponent* OldTarget, USingularisInteractionComponent* NewTarget);

	/** 交互者发起一次交互请求。 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互者视图接口",
		meta = (DisplayName = "交互触发")
	)
	void OnTriggered(USingularisInteractionComponent* Target, FGameplayTag StrategyTag);
};
