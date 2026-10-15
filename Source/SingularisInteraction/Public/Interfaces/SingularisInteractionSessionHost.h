/* ====================================================================== *
 * SingularisInteractionSessionHost.h                                     *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2026 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2026 TrifingZW. All Rights Reserved.                     *
 * Created: 2026/10/13 | Author: TrifingZW                                *
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

#include "SingularisInteractionSessionHost.generated.h"

class USingularisInteractionSession;

/**
 * 引力奇点交互会话宿主接口。
 *
 * 会话发射通道的承接方：会话端（交互组件，会话结算）与
 * 镜像端（交互者组件，本地会话）实现同一契约，会话对二者无感。
 */
UINTERFACE(Blueprintable, BlueprintType)
class USingularisInteractionSessionHost : public UInterface
{
	GENERATED_BODY()
};

/** 会话宿主实现接口：承接会话发出的会话事件、达成与中止。 */
class SINGULARISINTERACTION_API ISingularisInteractionSessionHost
{
	GENERATED_BODY()

public:
	/**
	 * 会话发射会话事件。
	 *
	 * @param Session 发射事件的会话实例。
	 * @param EventTag 事件标签。
	 * @param Alpha 归一化量值 0..1。
	 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互会话宿主接口",
		meta = (DisplayName = "发射会话事件")
	)
	void HostEmitSessionEvent(USingularisInteractionSession* Session, const FGameplayTag& EventTag, float Alpha);

	/**
	 * 会话完成会话。
	 *
	 * @param Session 完成会话的会话实例。
	 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互会话宿主接口",
		meta = (DisplayName = "完成会话")
	)
	void HostCompleteSession(USingularisInteractionSession* Session);

	/**
	 * 会话取消会话。
	 *
	 * @param Session 取消会话的会话实例。
	 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互会话宿主接口",
		meta = (DisplayName = "取消会话")
	)
	void HostCancelSession(USingularisInteractionSession* Session);
};
