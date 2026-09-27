/* ====================================================================== *
 * InteractionQueryer.h                                                   *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2025 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2025 TrifingZW. All Rights Reserved.                     *
 * Created: 2025/11/04 | Author: TrifingZW                                *
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

#include "Types/SingularisInteractionQueryerType.h"
#include "SingularisInteractionQueryer.generated.h"

class USingularisInteractionComponent;

/**
 * 引力奇点交互查询器。
 *
 * 交互目标的锁定策略：按射线距离与查询模式在玩家视线方向搜索可交互组件，
 * 输出查询结果供交互者组件维护悬浮状态与输入映射上下文。
 * 子类覆写 Query 实现自定义搜索规则。
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew, CollapseCategories)
class SINGULARISINTERACTION_API USingularisInteractionQueryer : public UObject
{
	GENERATED_BODY()

public:
#pragma region Parameter

	/** 射线距离（厘米） */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点交互查询器",
		meta = (DisplayName = "射线距离")
	)
	float TraceDistance = 200.0f;

	/** 查询模式 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点交互查询器",
		meta = (DisplayName = "查询模式")
	)
	ESingularisInteractionMode SingularisInteractionMode = ESingularisInteractionMode::Ray;

#pragma endregion

#pragma region SPI

	/**
	 * 执行一次交互查询。
	 *
	 * @param QueryerResult 输出的查询结果。
	 * @param QueryerParams 查询参数，承载视线与忽略目标。
	 * @return 命中有效可交互目标时返回 true。
	 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互查询器|SPI",
		meta = (DisplayName = "查询")
	)
	bool Query(
		FSingularisInteractionQueryerResult& QueryerResult,
		const FSingularisInteractionQueryerParams& QueryerParams
	);

#pragma endregion

private:
#pragma region Internal Function

	/**
	 * 反查碰撞组件登记的交互组件。
	 *
	 * @param Actor 命中碰撞组件所属的 Actor。
	 * @param PrimitiveComponent 命中的碰撞组件。
	 * @return 已登记的交互组件；未登记时返回 nullptr。
	 */
	static USingularisInteractionComponent* FindInteractionComponent(
		const AActor* Actor,
		UPrimitiveComponent* PrimitiveComponent
	);

#pragma endregion
};
