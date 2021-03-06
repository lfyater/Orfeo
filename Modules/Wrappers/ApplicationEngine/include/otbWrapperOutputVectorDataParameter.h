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

#ifndef otbWrapperOutputVectorDataParameter_h
#define otbWrapperOutputVectorDataParameter_h

#include "otbWrapperParameter.h"

#include "otbVectorData.h"
#include "otbVectorDataFileWriter.h"

namespace otb
{
namespace Wrapper
{
/** \class OutputVectorDataParameter
 *  \brief This class represents a OutputVectorData parameter
 *
 * \ingroup OTBApplicationEngine
 */

class OTBApplicationEngine_EXPORT OutputVectorDataParameter : public Parameter
{
public:
  /** Standard class typedef */
  typedef OutputVectorDataParameter      Self;
  typedef Parameter                     Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Defining ::New() static method */
  itkNewMacro(Self);

  /** RTTI support */
  itkTypeMacro(OutputVectorDataParameter, Parameter);


  /** Set the value */
  itkSetObjectMacro(VectorData, VectorDataType);

  /** Get the value */
  itkGetObjectMacro(VectorData, VectorDataType);

  /** Return true if a filename is set */
  bool HasValue() const ITK_OVERRIDE
  {
    std::string filename(this->GetFileName());
    return !filename.empty();
  }

  /** Return any value */
  void SetValue(VectorDataType* vd)
  {
    m_VectorData = vd;
    SetActive(true);
  }

  /** Return any value */
  VectorDataType* GetValue( void )
  {
    return m_VectorData;
  }

  void SetFileName (const char* filename)
  {
    m_FileName = filename;
    SetActive(true);
  }
  void SetFileName (const std::string& filename)
  {
    this->SetFileName(filename.c_str());
  }

  itkGetStringMacro(FileName);

  void Write()
  {
    m_Writer->SetFileName(m_FileName);
    m_Writer->SetInput(m_VectorData);
    m_Writer->Update();
  }

  itk::ProcessObject* GetWriter()
  {
    return m_Writer;
  }

  void InitializeWriters()
  {
    m_Writer = otb::VectorDataFileWriter<VectorDataType>::New();
  }

protected:
  /** Constructor */
  OutputVectorDataParameter()
  {
    this->SetName("Output Vector Data");
    this->SetKey("outvd");
  }

  /** Destructor */
  ~OutputVectorDataParameter() ITK_OVERRIDE
  {}


  VectorDataType::Pointer m_VectorData;
  std::string m_FileName;

  otb::VectorDataFileWriter<VectorDataType>::Pointer m_Writer;

private:
  OutputVectorDataParameter(const Parameter &); //purposely not implemented
  void operator =(const Parameter&); //purposely not implemented

};

} // End namespace Wrapper
} // End namespace otb

#endif
