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
 * 引力奇点交互查询器
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew, CollapseCategories)
class SINGULARISINTERACTION_API USingularisInteractionQueryer : public UObject
{
	GENERATED_BODY()

public:
#pragma region Parameter

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "SingularisInteraction|引力奇点交互查询器|参数",
		meta = (DisplayName = "射线距离")
	)
	float TraceDistance = 200.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "SingularisInteraction|引力奇点交互查询器|参数",
		meta = (DisplayName = "查询模式")
	)
	ESingularisInteractionMode SingularisInteractionMode = ESingularisInteractionMode::Ray;

#pragma endregion

#pragma region SPI

	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "SingularisInteraction|引力奇点交互查询器|SPI",
		meta = (DisplayName = "查询")
	)
	bool Query(
		FSingularisInteractionQueryerResult& QueryerResult,
		const FSingularisInteractionQueryerParams& QueryerParams
	);

#pragma endregion

private:
#pragma region Internal Function

	static USingularisInteractionComponent* FindInteractionComponent(
		const AActor* Actor,
		UPrimitiveComponent* PrimitiveComponent
	);

#pragma endregion
};
