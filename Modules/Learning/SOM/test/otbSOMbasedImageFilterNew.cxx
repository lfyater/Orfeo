/*
 * Copyright (C) 2005-2017 Centre National d'Etudes Spatiales (CNES)
 * Copyright (C) 2007-2012 Institut Mines Telecom / Telecom Bretagne
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


#include "otbVectorImage.h"
#include "itkVariableLengthVector.h"

#include "otbFlexibleDistanceWithMissingValue.h"
#include "otbSOMbasedImageFilter.h"

int otbSOMbasedImageFilterNew(int itkNotUsed(argc), char * itkNotUsed(argv) [])
{
  const unsigned int Dimension = 2;
  typedef double                                 PixelType;
  typedef otb::VectorImage<PixelType, Dimension> ImageType;
  typedef ImageType::PixelType                   VectorType;

  typedef otb::Statistics::FlexibleDistanceWithMissingValue<VectorType> DistanceType;

  typedef otb::SOMbasedImageFilter<ImageType, ImageType, DistanceType, ImageType>
  FilterType;
  FilterType::Pointer somFilter = FilterType::New();

  std::cout << somFilter << std::endl;

  return EXIT_SUCCESS;
}
