/* ====================================================================== *
 * SingularisInteractionWidgetComponent.cpp                               *
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

#include "Components/SingularisInteractionWidgetComponent.h"

#include <Components/ShapeComponent.h>
#include <Components/WidgetComponent.h>
#include <GameFramework/Pawn.h>
#include <UObject/ConstructorHelpers.h>

#include "Components/SingularisInteractionComponent.h"
#include "Components/SingularisInteractorComponent.h"
#include "Widgets/SingularisInteractionWidget.h"

#define ECC_INTERACTION ECC_GameTraceChannel1

USingularisInteractionWidgetComponent::USingularisInteractionWidgetComponent()
{
	SetIsReplicatedByDefault(false);

	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	bAutoActivate = true;

	static ConstructorHelpers::FClassFinder<USingularisInteractionWidget> DefaultWidgetClassFinder(
		TEXT(
			"/SingularisInteraction/UserInterfaces/WBP_Default_SingularisInteractionWidget.WBP_Default_SingularisInteractionWidget_C"
		)
	);

	if (DefaultWidgetClassFinder.Succeeded())
		InteractionWidgetClass = DefaultWidgetClassFinder.Class;
}

void USingularisInteractionWidgetComponent::BeginPlay()
{
	Super::BeginPlay();

	ProxyWidgetComponent();
	ProxyPromptVolume();
	ObserveInteractionComponent();
}

void USingularisInteractionWidgetComponent::ProxyWidgetComponent()
{
	const APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!IsValid(PlayerController) || !PlayerController->IsLocalController()) return;

	UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(WidgetComponentReference.GetComponent(GetOwner()));
	if (!IsValid(WidgetComponent)) return;

	InteractionWidget = CreateWidget<USingularisInteractionWidget>(GetWorld(), InteractionWidgetClass);
	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComponent->SetWidget(InteractionWidget);
	WidgetComponent->SetCollisionResponseToChannel(ECC_INTERACTION, ECR_Ignore);
}

void USingularisInteractionWidgetComponent::ProxyPromptVolume()
{
	const APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!IsValid(PlayerController) || !PlayerController->IsLocalController()) return;

	UShapeComponent* PromptVolume = Cast<UShapeComponent>(PromptVolumeReference.GetComponent(GetOwner()));
	if (!IsValid(PromptVolume)) return;

	PromptVolume->SetCollisionResponseToChannel(ECC_INTERACTION, ECR_Ignore);
	PromptVolume->OnComponentBeginOverlap.AddDynamic(
		this,
		&USingularisInteractionWidgetComponent::OnPromptVolumeBeginOverlap
	);
	PromptVolume->OnComponentEndOverlap.AddDynamic(
		this,
		&USingularisInteractionWidgetComponent::OnPromptVolumeEndOverlap
	);
}

void USingularisInteractionWidgetComponent::ObserveInteractionComponent()
{
	const APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!IsValid(PlayerController) || !PlayerController->IsLocalController()) return;

	USingularisInteractionComponent* InteractionComponent = Cast<USingularisInteractionComponent>(
		InteractionComponentReference.GetComponent(GetOwner())
	);
	if (!IsValid(InteractionComponent)) return;

	InteractionComponent->OnInteractionEvent.AddDynamic(
		InteractionWidget,
		&USingularisInteractionWidget::Trigger
	);
	InteractionComponent->OnInteractionHoverEvent.AddDynamic(
		InteractionWidget,
		&USingularisInteractionWidget::Hover
	);
	InteractionComponent->OnInteractionUnhoverEvent.AddDynamic(
		InteractionWidget,
		&USingularisInteractionWidget::Unhover
	);
}

// ReSharper disable CppMemberFunctionMayBeConst

void USingularisInteractionWidgetComponent::OnPromptVolumeBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (!IsValid(Pawn)) return;

	const APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
	if (!IsValid(PlayerController) || !PlayerController->IsLocalController()) return;

	const USingularisInteractorComponent* InteractorComponent =
		PlayerController->FindComponentByClass<USingularisInteractorComponent>();
	if (!IsValid(InteractorComponent)) return;

	InteractionWidget->EnterRange();
}

void USingularisInteractionWidgetComponent::OnPromptVolumeEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex
)
{
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (!IsValid(Pawn)) return;

	const APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
	if (!IsValid(PlayerController) || !PlayerController->IsLocalController()) return;

	const USingularisInteractorComponent* InteractorComponent =
		PlayerController->FindComponentByClass<USingularisInteractorComponent>();
	if (!IsValid(InteractorComponent)) return;

	InteractionWidget->ExitRange();
}

// ReSharper restore CppMemberFunctionMayBeConst
