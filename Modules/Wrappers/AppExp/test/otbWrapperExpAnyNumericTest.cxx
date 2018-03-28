/*
 * Copyright (C) 2005-2017 Centre National d'Etudes Spatiales (CNES)
 *
 * This file is part of Orfeo Toolbox
 *
 *     https://www.orfeo-toolbox.org/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "otbWrapperExpAnyNumeric.h"
using namespace otb::WrapperExp;
int AnyNumericTest( int  , char **  )
{
  any_numeric a = 3;
  any_numeric b (a);
  b = 5.5;
  return EXIT_SUCCESS;
}

int AnyNumericComp( int  , char **  )
{
  bool ret = true ;
  any_numeric a = 3;
  ret = ret && ( a == 3 );
  a = 3.0;
  ret = ret && ( a == 3 );
  a = 2.5 ;
  ret = ret && ( a != 3.0 );
  ret = ret && ( a > 2 );
  ret = ret && ( a >= 2.5 );
  ret = ret && ( a <= 2.5 );
  ret = ret && ( a < 6 );
  any_numeric b = -6;
  ret = ret && ( b < a );
  ret = ret && ( b != a );
  a = 3;
  b = 3.0;
  ret = ret && ( b == a );

  return ret;
}
