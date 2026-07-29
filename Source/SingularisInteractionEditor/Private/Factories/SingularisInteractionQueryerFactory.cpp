/* ====================================================================== *
 * SingularisInteractionQueryerFactory.cpp                                *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2026 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2026 TrifingZW. All Rights Reserved.                     *
 * Created: 2026/01/10 | Author: TrifingZW                                *
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

#include "Factories/SingularisInteractionQueryerFactory.h"

#include <Kismet2/KismetEditorUtilities.h>
#include <Objects/SingularisInteractionQueryer.h>

USingularisInteractionQueryerFactory::USingularisInteractionQueryerFactory()
{
	// 关键配置
	bCreateNew = true; // 允许创建新资产
	bEditAfterNew = true; // 创建后自动打开编辑器（可选）
	SupportedClass = USingularisInteractionQueryer::StaticClass(); // 关联具体的资产类
}

UObject* USingularisInteractionQueryerFactory::FactoryCreateNew(
	UClass* InClass,
	UObject* InParent,
	const FName InName,
	const EObjectFlags Flags,
	UObject* Context,
	FFeedbackContext* Warn
)
{
	// 核心逻辑：创建蓝图，并指定 ParentClass 为你的 C++ 抽象类
	// UMyAbstractClass 是你想要继承的那个 C++ 类
	return FKismetEditorUtilities::CreateBlueprint(
		USingularisInteractionQueryer::StaticClass(), 
		InParent, 
		InName, 
		BPTYPE_Normal, 
		UBlueprint::StaticClass(), 
		UBlueprintGeneratedClass::StaticClass(), 
		NAME_None
	);
}

bool USingularisInteractionQueryerFactory::ShouldShowInNewMenu() const
{
	return true;
}
