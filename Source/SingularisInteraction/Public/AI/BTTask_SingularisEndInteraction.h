/* ====================================================================== *
 * BTTask_SingularisEndInteraction.h                                      *
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
#include <BehaviorTree/BTTaskNode.h>
#include <BehaviorTree/BehaviorTreeTypes.h>

#include "BTTask_SingularisEndInteraction.generated.h"

class UBehaviorTreeComponent;

/**
 * 行为树任务：撤回一次交互。
 *
 * 提交撤回意图后立即完成；长按类会话提前松手即中止，连点类会话交由其窗口裁决。
 * 挂载要求：AIController 或其 Pawn 挂有交互者组件。
 */
UCLASS(meta = (DisplayName = "Singularis 撤回交互"))
class SINGULARISINTERACTION_API UBTTask_SingularisEndInteraction : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SingularisEndInteraction();

	/** 会话对应的响应标签 */
	UPROPERTY(
		EditAnywhere,
		Category = "引力奇点交互",
		meta = (
			Categories = "Singularis.Interaction.Response",
			ForceSelection = "true"
		)
	)
	FGameplayTag ResponseTag{};

	/** 目标黑板键（交互组件或 Actor） */
	UPROPERTY(EditAnywhere, Category = "引力奇点交互")
	FBlackboardKeySelector TargetKey{};

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};
