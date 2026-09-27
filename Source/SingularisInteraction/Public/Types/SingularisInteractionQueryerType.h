/* ====================================================================== *
 * SingularisInteractionQueryerType.h                                     *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2026 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2026 TrifingZW. All Rights Reserved.                     *
 * Created: 2026/01/22 | Author: TrifingZW                                *
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
#include <Engine/NetSerialization.h>
#include <GameFramework/Actor.h>

#include "Components/SingularisInteractionComponent.h"
#include "SingularisInteractionQueryerType.generated.h"

/**
 * 引力奇点交互查询器参数。
 */
USTRUCT(BlueprintType)
struct SINGULARISINTERACTION_API FSingularisInteractionQueryerParams
{
	GENERATED_BODY()

	/** 视线起点（世界坐标） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ViewLoc{};

	/** 视线朝向 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator ViewRot{};

	/** 需忽略的 Actor，通常为发起查询的玩家 Pawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* IgnoredActor = nullptr;
};

/**
 * 引力奇点交互查询器结果。
 */
USTRUCT(BlueprintType)
struct SINGULARISINTERACTION_API FSingularisInteractionQueryerResult
{
	GENERATED_BODY()

	/** 命中的 Actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* InteractionActor = nullptr;

	/** 命中 Actor 上登记的交互组件 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USingularisInteractionComponent* InteractionComponent = nullptr;

	/** 命中点（世界坐标） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector_NetQuantize ImpactPoint{};

	/**
	 * 结果是否指向有效的交互目标。
	 *
	 * @return Actor 与交互组件均有效时返回 true。
	 */
	bool IsInteractionValid() const
	{
		return IsValid(InteractionActor) && IsValid(InteractionComponent);
	}

	/**
	 * 比较两个查询结果是否指向同一交互目标。
	 *
	 * @param Other 待比较的查询结果。
	 * @return 交互目标一致时返回 true。
	 */
	bool operator==(const FSingularisInteractionQueryerResult& Other) const
	{
		return InteractionActor == Other.InteractionActor &&
			InteractionComponent == Other.InteractionComponent;
	}
};
