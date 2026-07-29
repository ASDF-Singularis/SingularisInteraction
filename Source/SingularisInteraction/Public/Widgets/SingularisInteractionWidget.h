/* ====================================================================== *
 * SingularisInteractionWidget.h                                          *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2026 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2026 TrifingZW. All Rights Reserved.                     *
 * Created: 2026/01/05 | Author: TrifingZW                                *
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
#include <Blueprint/UserWidget.h>

#include "SingularisInteractionWidget.generated.h"

/**
 * 引力奇点交互控件
 */
UCLASS(Abstract, Blueprintable)
class SINGULARISINTERACTION_API USingularisInteractionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
#pragma region SPI

	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "SingularisInteraction|引力奇点交互|SPI",
		meta = (DisplayName = "Trigger")
	)
	void Trigger();

	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "SingularisInteraction|引力奇点交互|SPI",
		meta = (DisplayName = "Hover")
	)
	void Hover();

	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "SingularisInteraction|引力奇点交互|SPI",
		meta = (DisplayName = "Unhover")
	)
	void Unhover();

	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "SingularisInteraction|引力奇点交互|SPI",
		meta = (DisplayName = "EnterRange")
	)
	void EnterRange();

	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "SingularisInteraction|引力奇点交互|SPI",
		meta = (DisplayName = "ExitRange")
	)
	void ExitRange();

#pragma endregion
};
