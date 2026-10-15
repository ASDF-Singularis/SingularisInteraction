/* ====================================================================== *
 * SingularisInteractionValueType.cpp                                     *
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

#include "Types/SingularisInteractionValueType.h"

float FSingularisInteractionValue::GetMagnitude() const
{
	// 1) 布尔归一化为 0 / 1，其余取最大分量绝对值
	if (IsBoolean())
		return Value.X != 0.0 ? 1.0f : 0.0f;

	return static_cast<float>(Value.GetAbsMax());
}

float FSingularisInteractionValue::GetScalar() const
{
	// 1) 布尔归一化为 0 / 1，其余取首分量（带符号）
	if (IsBoolean())
		return Value.X != 0.0 ? 1.0f : 0.0f;

	return static_cast<float>(Value.X);
}

FVector2D FSingularisInteractionValue::GetVector2D() const { return FVector2D(Value.X, Value.Y); }
