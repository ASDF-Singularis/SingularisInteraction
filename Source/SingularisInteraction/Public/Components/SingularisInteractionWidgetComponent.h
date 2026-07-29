/* ====================================================================== *
 * SingularisInteractionWidgetComponent.h                                 *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2026 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2026 TrifingZW. All Rights Reserved.                     *
 * Created: 2026/01/30 | Author: TrifingZW                                *
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
#include <Components/ActorComponent.h>

#include "SingularisInteractionWidgetComponent.generated.h"

struct FHitResult;
class USingularisInteractionWidget;

UCLASS(
	Blueprintable,
	BlueprintType,
	ClassGroup = ("Singularis"),
	meta = (BlueprintSpawnableComponent, DisplayName = "引力奇点交互控件组件")
)
class SINGULARISINTERACTION_API USingularisInteractionWidgetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
#pragma region Instantiation

	UPROPERTY(
		EditInstanceOnly,
		BlueprintReadOnly,
		Category = "SingularisInteraction|引力奇点交互控件|Instantiation",
		meta = (DisplayName = "交互控件")
	)
	USingularisInteractionWidget* InteractionWidget = nullptr;

#pragma endregion

#pragma region Parameter

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "SingularisInteraction|引力奇点交互控件|引用",
		meta = (DisplayName = "控件组件引用", UseComponentPicker, AllowedClasses = "/Script/Engine.WidgetComponent")

	)
	FComponentReference WidgetComponentReference{};

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "SingularisInteraction|引力奇点交互控件|引用",
		meta = (DisplayName = "提示范围引用", UseComponentPicker, AllowedClasses = "/Script/Engine.ShapeComponent")
	)
	FComponentReference PromptVolumeReference{};

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "SingularisInteraction|引力奇点交互控件|引用",
		meta = (
			DisplayName = "交互组件引用",
			UseComponentPicker,
			AllowedClasses = "/Script/SingularisInteraction.SingularisInteractionComponent"
		)
	)
	FComponentReference InteractionComponentReference{};

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "SingularisInteraction|引力奇点交互控件|参数",
		meta = (DisplayName = "交互控件类")
	)
	TSubclassOf<USingularisInteractionWidget> InteractionWidgetClass = nullptr;

#pragma endregion

#pragma region Constructors

	USingularisInteractionWidgetComponent();

#pragma endregion

#pragma region ActorComponent Interface

	virtual void BeginPlay() override;

#pragma endregion

private:
#pragma region Internal Function

	void ProxyWidgetComponent();
	void ProxyPromptVolume();
	void ObserveInteractionComponent();

#pragma endregion

#pragma region Callback

	UFUNCTION()
	void OnPromptVolumeBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnPromptVolumeEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

#pragma endregion
};
