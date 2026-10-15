/* ====================================================================== *
 * SingularisInteractionValueType.h                                       *
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

#include "SingularisInteractionValueType.generated.h"

/**
 * 引力奇点交互量值类型。
 *
 * 意图载荷的维度声明：决定消费方如何解释量值，与输入设备无关。
 */
UENUM(BlueprintType)
enum class ESingularisInteractionValueType : uint8
{
	/** 布尔：仅有无 */
	Boolean UMETA(DisplayName = "布尔"),

	/** 一维：带符号标量（轴值 / 力度） */
	Axis1D UMETA(DisplayName = "一维"),

	/** 二维：平面向量（屏幕位移 / 摇杆） */
	Axis2D UMETA(DisplayName = "二维"),

	/** 三维：空间向量（世界位移 / 拖拽） */
	Axis3D UMETA(DisplayName = "三维"),
};

/**
 * 引力奇点交互量值。
 *
 * 与输入设备无关的中性量值载体：控制端把任意来源的原始量值装箱为
 * 布尔 / 一维 / 二维 / 三维，逻辑端按自身机制解释，不携带任何输入语义。
 */
USTRUCT(BlueprintType)
struct SINGULARISINTERACTION_API FSingularisInteractionValue
{
	GENERATED_BODY()

	/** 值类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESingularisInteractionValueType ValueType = ESingularisInteractionValueType::Axis1D;

	/** 原始量值（布尔取 X；一维取 X；二维取 XY；三维取 XYZ） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Value = FVector::ZeroVector;

	/** 默认构造：一维零值。 */
	FSingularisInteractionValue() = default;

	/** 由布尔构造。 */
	explicit FSingularisInteractionValue(const bool bInValue)
		: ValueType(ESingularisInteractionValueType::Boolean), Value(bInValue ? 1.0 : 0.0, 0.0, 0.0) {}

	/** 由一维构造。 */
	explicit FSingularisInteractionValue(const float InValue)
		: ValueType(ESingularisInteractionValueType::Axis1D), Value(InValue, 0.0, 0.0) {}

	/** 由二维构造。 */
	explicit FSingularisInteractionValue(const FVector2D& InValue)
		: ValueType(ESingularisInteractionValueType::Axis2D), Value(InValue.X, InValue.Y, 0.0) {}

	/** 由三维构造。 */
	explicit FSingularisInteractionValue(const FVector& InValue)
		: ValueType(ESingularisInteractionValueType::Axis3D), Value(InValue) {}

	/** 是否布尔量值。 */
	bool IsBoolean() const { return ValueType == ESingularisInteractionValueType::Boolean; }

	/** 是否一维量值。 */
	bool IsAxis1D() const { return ValueType == ESingularisInteractionValueType::Axis1D; }

	/** 是否二维量值。 */
	bool IsAxis2D() const { return ValueType == ESingularisInteractionValueType::Axis2D; }

	/** 是否三维量值。 */
	bool IsAxis3D() const { return ValueType == ESingularisInteractionValueType::Axis3D; }

	/**
	 * 幅值（非负）：布尔归一化为 0 / 1，其余取最大分量的绝对值。
	 *
	 * 推导函数仅对 C++ 开放：`USTRUCT` 作用域不允许 `UFUNCTION`，蓝图侧直接读 `ValueType` / `Value`。
	 *
	 * @return 幅值。
	 */
	float GetMagnitude() const;

	/**
	 * 带符号标量：布尔归一化为 0 / 1，其余取首分量。
	 *
	 * 供标量型会话（轴积分 / 拖动 / 脉冲）消费。
	 *
	 * @return 标量。
	 */
	float GetScalar() const;

	/**
	 * 二维分量。
	 *
	 * @return 平面量值。
	 */
	FVector2D GetVector2D() const;
};
