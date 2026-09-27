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
#include <UObject/ScriptInterface.h>

#include "Interfaces/SingularisInteractionViewInterface.h"
#include "SingularisInteractionWidgetComponent.generated.h"

struct FHitResult;
class UUserWidget;

/**
 * 引力奇点交互控件组件。
 *
 * 屏幕空间交互观察者：创建交互视图、装配提示范围重叠回调，并订阅交互组件事件，
 * 将交互状态与事件经 ISingularisInteractionViewInterface（SPI）推送至视图。
 * 组件仅依赖接口而非具体控件类型，任意实现该接口的 UObject 均可作为视图接入。
 */
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
#pragma region Parameter

	/** 承载交互控件的控件组件引用 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点交互控件组件",
		meta = (DisplayName = "控件组件引用", UseComponentPicker, AllowedClasses = "/Script/UMG.WidgetComponent")
	)
	FComponentReference WidgetComponentReference{};

	/** 触发进入/离开范围反馈的提示范围引用 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点交互控件组件",
		meta = (DisplayName = "提示范围引用", UseComponentPicker, AllowedClasses = "/Script/Engine.ShapeComponent")
	)
	FComponentReference PromptVolumeReference{};

	/** 关联的交互组件引用 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点交互控件组件",
		meta = (
			DisplayName = "交互组件引用",
			UseComponentPicker,
			AllowedClasses = "/Script/SingularisInteraction.SingularisInteractionComponent"
		)
	)
	FComponentReference InteractionComponentReference{};

	/** 交互控件类 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点交互控件组件",
		meta = (
			DisplayName = "交互控件类",
			MustImplement = "/Script/SingularisInteraction.SingularisInteractionViewInterface"
		)
	)
	TSubclassOf<UUserWidget> InteractionWidgetClass = nullptr;

#pragma endregion

private:
#pragma region State

	/** 运行时实例化的交互视图缓存 */
	UPROPERTY(Transient)
	TScriptInterface<ISingularisInteractionViewInterface> InteractionView{};

#pragma endregion

public:
#pragma region Constructors

	USingularisInteractionWidgetComponent();

#pragma endregion

#pragma region ActorComponent Interface

	virtual void BeginPlay() override;

#pragma endregion

#pragma region API

	/**
	 * 获取运行时实例化的交互视图。
	 *
	 * @return 交互视图对象；尚未实例化时返回 nullptr。
	 */
	UFUNCTION(
		BlueprintPure,
		Category = "引力奇点交互控件组件|API",
		meta = (DisplayName = "GetInteractionView")
	)
	UObject* GetInteractionView() const { return InteractionView.GetObject(); }

#pragma endregion

private:
#pragma region Callback

	/** 交互触发回调：转发至视图 */
	UFUNCTION()
	void HandleInteraction() const;

	/** 交互悬浮回调：转发至视图 */
	UFUNCTION()
	void HandleHover() const;

	/** 交互未悬浮回调：转发至视图 */
	UFUNCTION()
	void HandleUnhover() const;

	/** 提示范围 BeginOverlap 回调：本地玩家进入范围时通知视图 */
	UFUNCTION()
	void OnPromptVolumeBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	/** 提示范围 EndOverlap 回调：本地玩家离开范围时通知视图 */
	UFUNCTION()
	void OnPromptVolumeEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

#pragma endregion

#pragma region Internal Function

	/** 在本地客户端实例化交互视图并挂载到控件组件 */
	void ProxyWidgetComponent();

	/** 装配提示范围的交互通道与重叠回调 */
	void ProxyPromptVolume();

	/** 绑定交互组件事件并推送一次全量状态 */
	void ObserveInteractionComponent();

#pragma endregion
};
