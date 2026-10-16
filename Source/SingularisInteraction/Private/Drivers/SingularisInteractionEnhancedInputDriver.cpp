/* ====================================================================== *
 * SingularisInteractionEnhancedInputDriver.cpp                           *
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

#include "Drivers/SingularisInteractionEnhancedInputDriver.h"

#include <EnhancedInputComponent.h>
#include <EnhancedInputSubsystems.h>
#include <InputAction.h>
#include <InputMappingContext.h>
#include <Engine/LocalPlayer.h>
#include <Engine/World.h>
#include <GameFramework/PlayerController.h>
#include <Math/UnrealMathUtility.h>
#include <UObject/ConstructorHelpers.h>

#include "SingularisInteraction.h"
#include "Components/SingularisInteractionComponent.h"
#include "Components/SingularisInteractorComponent.h"
#include "Types/SingularisInteractionGameplayTags.h"

/**
 * 把增强输入量值装箱为中立交互量值。
 *
 * @param ActionValue 增强输入量值。
 * @return 中立交互量值。
 */
static FSingularisInteractionValue ToInteractionValue(const FInputActionValue& ActionValue)
{
	switch (ActionValue.GetValueType())
	{
	case EInputActionValueType::Boolean:
		return FSingularisInteractionValue(ActionValue.Get<bool>());
	case EInputActionValueType::Axis2D:
		return FSingularisInteractionValue(ActionValue.Get<FVector2D>());
	case EInputActionValueType::Axis3D:
		return FSingularisInteractionValue(ActionValue.Get<FVector>());
	case EInputActionValueType::Axis1D:
	default:
		return FSingularisInteractionValue(ActionValue.Get<float>());
	}
}

USingularisInteractionEnhancedInputDriver::USingularisInteractionEnhancedInputDriver()
{
	static const ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultInputMappingContextFinder(
		TEXT("/SingularisInteraction/Inputs/IMC_Default_SingularisInteractor.IMC_Default_SingularisInteractor")
	);
	static const ConstructorHelpers::FObjectFinder<UInputAction> DefaultInteractionActionFinder(
		TEXT("/SingularisInteraction/Inputs/Actions/IA_Interaction.IA_Interaction")
	);

	if (DefaultInputMappingContextFinder.Succeeded())
		InputMappingContext = DefaultInputMappingContextFinder.Object;
	else
		UE_LOG(
		LogSingularisInteraction,
		Warning,
		TEXT("默认输入映射上下文加载失败：%s"),
		TEXT("/SingularisInteraction/Inputs/IMC_Default_SingularisInteractor")
	);

	if (DefaultInteractionActionFinder.Succeeded())
	{
		FSingularisInteractionInputBinding DefaultBinding;
		DefaultBinding.InputAction = DefaultInteractionActionFinder.Object;
		DefaultBinding.PipelineTag = SingularisInteraction_Pipeline_Default;
		InputBindings.Add(DefaultBinding);
	}
	else
		UE_LOG(
		LogSingularisInteraction,
		Warning,
		TEXT("默认交互输入动作加载失败：%s"),
		TEXT("/SingularisInteraction/Inputs/Actions/IA_Interaction")
	);
}

void USingularisInteractionEnhancedInputDriver::InitializeDriver_Implementation(
	USingularisInteractorComponent* InInteractorComponent
)
{
	Super::InitializeDriver_Implementation(InInteractorComponent);

	// 1) 解析本地玩家控制器（仅本地玩家交互者存在输入可绑）
	APlayerController* PlayerController = IsValid(InInteractorComponent)
		                                      ? InInteractorComponent->GetInteractorPlayerController()
		                                      : nullptr;
	if (!IsValid(PlayerController))
		return;

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
	if (!IsValid(EnhancedInputComponent))
	{
		UE_LOG(
			LogSingularisInteraction,
			Warning,
			TEXT("[%s] InitializeDriver：EnhancedInputComponent 无效，交互输入不可用"),
			*GetNameSafe(PlayerController)
		);
		return;
	}

	// 2) 逐条绑定动作（按下 / 松开 / 强制取消 / 连续值）
	for (const FSingularisInteractionInputBinding& Binding : InputBindings)
	{
		if (!IsValid(Binding.InputAction) || !Binding.PipelineTag.IsValid())
			continue;

		EnhancedInputComponent->BindAction(
			Binding.InputAction,
			ETriggerEvent::Started,
			this,
			&USingularisInteractionEnhancedInputDriver::HandleActionStarted,
			Binding.PipelineTag
		);
		EnhancedInputComponent->BindAction(
			Binding.InputAction,
			ETriggerEvent::Completed,
			this,
			&USingularisInteractionEnhancedInputDriver::HandleActionReleased,
			Binding.PipelineTag
		);
		EnhancedInputComponent->BindAction(
			Binding.InputAction,
			ETriggerEvent::Canceled,
			this,
			&USingularisInteractionEnhancedInputDriver::HandleActionReleased,
			Binding.PipelineTag
		);

		if (Binding.bCarryValue)
		{
			EnhancedInputComponent->BindAction(
				Binding.InputAction,
				ETriggerEvent::Triggered,
				this,
				&USingularisInteractionEnhancedInputDriver::HandleActionValue,
				Binding.PipelineTag
			);
		}
	}

	// 3) 刷新输入映射上下文
	RefreshInputMappingContext();

	UE_LOG(
		LogSingularisInteraction,
		Display,
		TEXT("[%s] InitializeDriver：增强输入驱动器装配完成，绑定动作 %d 条"),
		*GetNameSafe(PlayerController),
		InputBindings.Num()
	);
}

void USingularisInteractionEnhancedInputDriver::DeinitializeDriver_Implementation()
{
	// 1) 卸载输入映射上下文（动作绑定随组件销毁自动失效）
	APlayerController* PlayerController = IsValid(GetInteractorComponent())
		                                      ? GetInteractorComponent()->GetInteractorPlayerController()
		                                      : nullptr;
	if (!IsValid(PlayerController) || !IsValid(InputMappingContext) || !PlayerController->IsLocalController())
		return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (IsValid(Subsystem))
		Subsystem->RemoveMappingContext(InputMappingContext);

	Super::DeinitializeDriver_Implementation();
}

void USingularisInteractionEnhancedInputDriver::OnTargetChanged_Implementation(
	USingularisInteractionComponent* OldTarget,
	USingularisInteractionComponent* NewTarget
)
{
	// 1) 目标锁定状态变更驱动映射上下文挂卸
	RefreshInputMappingContext();
}

// ReSharper disable CppMemberFunctionMayBeConst

void USingularisInteractionEnhancedInputDriver::HandleActionStarted(
	const FInputActionValue& ActionValue,
	const FGameplayTag PipelineTag
)
{
	// 1) 提交主张意图（量值携带力度）
	SubmitIntent(PipelineTag, SingularisInteraction_Intent_Press, ToInteractionValue(ActionValue));
}

void USingularisInteractionEnhancedInputDriver::HandleActionReleased(
	const FInputActionValue& ActionValue,
	const FGameplayTag PipelineTag
)
{
	// 1) 提交撤回意图
	SubmitIntent(PipelineTag, SingularisInteraction_Intent_Release, FSingularisInteractionValue());
}

void USingularisInteractionEnhancedInputDriver::HandleActionValue(
	const FInputActionValue& ActionValue,
	const FGameplayTag PipelineTag
)
{
	// 1) 节流与死区判据：间隔未达且量值变化细微时不提交（标量型取带符号分量，其余取幅值，避免正负翻向被吞）
	const UWorld* World = GetWorld();
	const float Now = IsValid(World) ? World->GetTimeSeconds() : 0.0f;
	const FSingularisInteractionValue Sample = ToInteractionValue(ActionValue);
	const float Value = Sample.IsAxis1D() ? Sample.GetScalar() : Sample.GetMagnitude();

	FSingularisInteractionValueTracking& Tracking = ValueTracking.FindOrAdd(PipelineTag);
	if (Now - Tracking.Time < ValueSubmitInterval && FMath::Abs(Value - Tracking.Value) < ValueDeadband)
		return;

	Tracking.Time = Now;
	Tracking.Value = Value;

	// 2) 提交连续值意图
	SubmitIntent(PipelineTag, SingularisInteraction_Intent_Value, Sample);
}

// ReSharper restore CppMemberFunctionMayBeConst

void USingularisInteractionEnhancedInputDriver::SubmitIntent(
	const FGameplayTag PipelineTag,
	const FGameplayTag IntentTag,
	const FSingularisInteractionValue& Value
) const
{
	// 1) 经宿主交互者组件单一入口提交（内部承担目标解析、镜像派生与提交）
	if (!IsValid(GetInteractorComponent()))
		return;

	GetInteractorComponent()->SubmitInteractionIntent(
		GetInteractorComponent()->GetCurrentTarget(),
		PipelineTag,
		IntentTag,
		Value
	);
}

void USingularisInteractionEnhancedInputDriver::RefreshInputMappingContext() const
{
	// 1) 基础检查
	APlayerController* PlayerController = IsValid(GetInteractorComponent())
		                                      ? GetInteractorComponent()->GetInteractorPlayerController()
		                                      : nullptr;
	if (!IsValid(PlayerController) || !PlayerController->IsLocalController())
		return;
	if (!IsValid(InputMappingContext))
		return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (!IsValid(Subsystem))
	{
		UE_LOG(
			LogSingularisInteraction,
			Warning,
			TEXT("[%s] RefreshInputMappingContext：EnhancedInput 本地玩家子系统无效"),
			*GetNameSafe(PlayerController)
		);
		return;
	}

	// 2) 依据目标锁定状态挂载或卸载映射上下文
	if (IsValid(GetInteractorComponent()->GetCurrentTarget()))
		Subsystem->AddMappingContext(InputMappingContext, InputPriority);
	else
		Subsystem->RemoveMappingContext(InputMappingContext);
}
