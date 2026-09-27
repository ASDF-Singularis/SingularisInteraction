/* ====================================================================== *
 * InteractionQueryer.cpp                                                 *
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

#include "Objects/SingularisInteractionQueryer.h"

#include <CollisionQueryParams.h>
#include <Components/PrimitiveComponent.h>
#include <Engine/HitResult.h>
#include <Engine/World.h>

#include "Components/SingularisInteractionComponent.h"
#include "Subsystems/SingularisInteractionSubsystem.h"

#define ECC_INTERACTION ECC_GameTraceChannel1

bool USingularisInteractionQueryer::Query_Implementation(
	FSingularisInteractionQueryerResult& QueryerResult,
	const FSingularisInteractionQueryerParams& QueryerParams
)
{
	// 1) 计算射线起点和终点
	const FVector Start = QueryerParams.ViewLoc;
	const FVector Forward = QueryerParams.ViewRot.Vector();
	const FVector End = Start + Forward * TraceDistance;

	// 2) 配置查询参数
	FCollisionQueryParams Params(SCENE_QUERY_STAT(LineTrace), true);
	Params.AddIgnoredActor(QueryerParams.IgnoredActor);

	// 3) 执行 LineTrace
	FHitResult HitResult;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_INTERACTION, Params);

	if (!bHit) return false;

	// 4) 命中校验与结果装配
	UPrimitiveComponent* PrimitiveComponent = HitResult.GetComponent();
	if (!IsValid(PrimitiveComponent)) return false;

	AActor* Actor = PrimitiveComponent->GetOwner();
	if (!IsValid(Actor)) return false;

	USingularisInteractionComponent* InteractionComponent = FindInteractionComponent(Actor, PrimitiveComponent);
	if (!IsValid(InteractionComponent)) return false;

	QueryerResult.InteractionActor = Actor;
	QueryerResult.InteractionComponent = InteractionComponent;
	QueryerResult.ImpactPoint = HitResult.ImpactPoint;

	return true;
}

USingularisInteractionComponent* USingularisInteractionQueryer::FindInteractionComponent(
	const AActor* Actor,
	UPrimitiveComponent* PrimitiveComponent
)
{
	if (!IsValid(Actor) || !IsValid(PrimitiveComponent)) return nullptr;

	const UWorld* World = Actor->GetWorld();
	if (!IsValid(World)) return nullptr;

	USingularisInteractionSubsystem* MappingSubsystem =
		World->GetSubsystem<USingularisInteractionSubsystem>();
	if (!IsValid(MappingSubsystem)) return nullptr;

	return MappingSubsystem->MappingComponent(PrimitiveComponent);
}
