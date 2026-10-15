/* ====================================================================== *
 * SingularisInteractionDriver.h                                          *
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
#include <UObject/Object.h>

#include "SingularisInteractionDriver.generated.h"

class USingularisInteractorComponent;
class USingularisInteractionComponent;

/**
 * 引力奇点交互驱动器。
 *
 * 输入源适配基类：把某种来源（增强输入 / UI / 回放 / 测试）翻译为中性交互意图。
 * 驱动器是控制端的结构扩展点——协议不假设意图如何产生，
 * AI 与脚本无需驱动器，直接经交互者组件 API 或交互子系统提交意图。
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, CollapseCategories)
class SINGULARISINTERACTION_API USingularisInteractionDriver : public UObject
{
	GENERATED_BODY()

#pragma region State

	/** 宿主交互者组件 */
	TWeakObjectPtr<USingularisInteractorComponent> InteractorComponent = nullptr;

#pragma endregion

public:
#pragma region API

	/**
	 * 宿主交互者组件（装配期绑定）。
	 *
	 * 供驱动器在蓝图或 C++ 中提交意图与读取宿主状态；未装配或宿主失效时返回 nullptr。
	 *
	 * @return 宿主交互者组件。
	 */
	UFUNCTION(
		BlueprintPure,
		Category = "引力奇点交互驱动器|API",
		meta = (DisplayName = "GetInteractorComponent")
	)
	USingularisInteractorComponent* GetInteractorComponent() const;

#pragma endregion

#pragma region SPI

	/**
	 * 装配：绑定宿主交互者组件。
	 *
	 * 由框架在交互者组件初始化时调用；驱动器在自身条件不足时应静默返回
	 * （如增强输入驱动器在非本地玩家交互者上无输入可绑）。
	 *
	 * @param InInteractorComponent 宿主交互者组件。
	 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互驱动器|SPI",
		meta = (DisplayName = "InitializeDriver")
	)
	void InitializeDriver(USingularisInteractorComponent* InInteractorComponent);

	/** 卸载：释放驱动器持有的外部装配 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互驱动器|SPI",
		meta = (DisplayName = "DeinitializeDriver")
	)
	void DeinitializeDriver();

	/**
	 * 目标变更通知：输入侧装配时机（如映射上下文挂卸）。
	 *
	 * @param OldTarget 旧目标组件；无旧目标时为 nullptr。
	 * @param NewTarget 新目标组件；无新目标时为 nullptr。
	 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互驱动器|SPI",
		meta = (DisplayName = "OnTargetChanged")
	)
	void OnTargetChanged(
		USingularisInteractionComponent* OldTarget,
		USingularisInteractionComponent* NewTarget
	);

#pragma endregion
};
