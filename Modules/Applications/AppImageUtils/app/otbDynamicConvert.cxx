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

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"

#include "otbVectorRescaleIntensityImageFilter.h"
#include "otbUnaryImageFunctorWithVectorImageFilter.h"
#include "otbStreamingShrinkImageFilter.h"
#include "itkListSample.h"
#include "otbListSampleToHistogramListGenerator.h"
#include "itkImageRegionConstIterator.h"

#include "otbImageListToVectorImageFilter.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "otbImageList.h"

#include <numeric>

namespace otb
{
namespace Wrapper
{

namespace Functor
{
  template< class TScalar >
class ITK_EXPORT LogFunctor
{
public:
  TScalar operator() (const TScalar& v) const
  {
    return std::log(v);
  }
};
} // end namespace Functor



class DynamicConvert : public Application
{
public:
  /** Standard class typedefs. */
  typedef DynamicConvert                Self;
  typedef Application                   Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Standard macro */
  itkNewMacro(Self);

  itkTypeMacro(DynamicConvert, otb::Application);

  /** Filters typedef */
  typedef itk::Statistics::ListSample<FloatVectorImageType::PixelType> ListSampleType;
  typedef itk::Statistics::DenseFrequencyContainer2 DFContainerType;
  typedef ListSampleToHistogramListGenerator<ListSampleType,
    FloatVectorImageType::InternalPixelType,
    DFContainerType> HistogramsGeneratorType;
    
  typedef StreamingShrinkImageFilter<FloatVectorImageType,
    FloatVectorImageType> ShrinkFilterType;

  typedef Functor::LogFunctor<FloatVectorImageType::InternalPixelType> TransferLogFunctor;
  typedef UnaryImageFunctorWithVectorImageFilter<FloatVectorImageType,
    FloatVectorImageType,
    TransferLogFunctor> TransferLogType;

private:

  void DoInit() ITK_OVERRIDE
  {
    SetName("DynamicConvert");
    SetDescription("Change the pixel type and rescale the image's dynamic");

    // Documentation
    SetDocName("Dynamic Conversion");
    // TODO
    SetDocLongDescription("This application performs an image pixel type "
      "conversion (short, ushort, uchar, int, uint, float and double types are "
      "handled). The output image is written in the specified format (ie. "
      "that corresponds to the given extension).\n The conversion can include "
      "a rescale of the data range, by default it's set between the 2nd to "
      "the 98th percentile. The rescale can be linear or log2. \n The choice "
      "of the output channels can be done with the extended filename, but "
      "less easy to handle. To do this, a 'channels' parameter allows you to "
      "select the desired bands at the output. There are 3 modes, the "
      "available choices are: \n * grayscale :  to display mono image as "
      "standard color image \n * rgb : select 3 bands in the input image "
      "(multi-bands) \n * all : keep all bands.");
    SetDocLimitations("None");
    SetDocAuthors("OTB-Team");
    SetDocSeeAlso("Convert, Rescale");

    AddDocTag(Tags::Manip);
    AddDocTag("Conversion");
    AddDocTag("Image Dynamic");

    AddParameter(ParameterType_InputImage, "in", "Input image");
    SetParameterDescription("in", "Input image");

    AddParameter(ParameterType_OutputImage, "out",  "Output Image");
    SetParameterDescription("out", "Output image");
    SetDefaultOutputPixelType("out",ImagePixelType_uint8);

    AddParameter(ParameterType_Choice, "type", "Rescale type");
    SetParameterDescription("type", "Transfer function for the rescaling");
    AddChoice("type.linear", "Linear");
    AddChoice("type.log2", "Log2");
    SetParameterString("type", "linear", false);

    AddParameter(ParameterType_Float,"type.linear.gamma",
      "Gamma correction factor");
    SetParameterDescription("type.linear.gamma",
      "Gamma correction factor");
    SetDefaultParameterFloat("type.linear.gamma",1.0);
    MandatoryOff("type.linear.gamma");

    AddParameter(ParameterType_InputImage,  "mask",   "Input mask");
    SetParameterDescription("mask",
      "The masked pixels won't be used to adapt the dynamic "
      "(the mask must have the same dimensions as the input image)");
    MandatoryOff("mask");
    DisableParameter("mask");

    AddParameter(ParameterType_Group,"quantile","Histogram quantile cutting");
    SetParameterDescription("quantile",
      "Cut the histogram edges before rescaling");

    AddParameter(ParameterType_Float, "quantile.high", "High cut quantile");
    SetParameterDescription("quantile.high", 
      "Quantiles to cut from histogram high values "
      "before computing min/max rescaling (in percent, 2 by default)");
    MandatoryOff("quantile.high");
    SetDefaultParameterFloat("quantile.high", 2.0);
    DisableParameter("quantile.high");

    AddParameter(ParameterType_Float, "quantile.low", "Low cut quantile");
    SetParameterDescription("quantile.low", 
      "Quantiles to cut from histogram low values "
      "before computing min/max rescaling (in percent, 2 by default)");
    MandatoryOff("quantile.low");
    SetDefaultParameterFloat("quantile.low", 2.0);
    DisableParameter("quantile.low");

    AddParameter(ParameterType_Choice, "channels", "Channels selection");
    SetParameterDescription("channels", "It's possible to select the channels "
      "of the output image. There are 3 modes, the available choices are:");

    AddChoice("channels.all", "Default mode");
    SetParameterDescription("channels.all", 
      "Select all bands in the input image, (1,...,n).");

    AddChoice("channels.grayscale", "Grayscale mode");
    SetParameterDescription("channels.grayscale", 
      "Display single channel as standard color image.");
    AddParameter(ParameterType_Int, "channels.grayscale.channel", 
      "Grayscale channel");
    SetDefaultParameterInt("channels.grayscale.channel", 1);
    SetMinimumParameterIntValue("channels.grayscale.channel", 1);

    AddChoice("channels.rgb", "RGB composition");
    SetParameterDescription("channels.rgb", "Select 3 bands in the input image "
      "(multi-bands), by default (1,2,3).");

    AddParameter(ParameterType_Int, "channels.rgb.red", "Red Channel");
    SetParameterDescription("channels.rgb.red", "Red channel index.");
    SetMinimumParameterIntValue("channels.rgb.red", 1);
    AddParameter(ParameterType_Int, "channels.rgb.green", "Green Channel");
    SetParameterDescription("channels.rgb.green", "Green channel index.");
    SetMinimumParameterIntValue("channels.rgb.green", 1);
    AddParameter(ParameterType_Int, "channels.rgb.blue", "Blue Channel");
    SetParameterDescription("channels.rgb.blue", "Blue channel index.");
    SetMinimumParameterIntValue("channels.rgb.blue", 1);

    AddParameter(ParameterType_Float, "outmin", "Output min value");
    SetDefaultParameterFloat("outmin", 0.0);
    SetParameterDescription( "outmin", "Minimum value of the output image." );
    AddParameter(ParameterType_Float, "outmax", "Output max value");
    SetDefaultParameterFloat("outmax", 255.0);
    SetParameterDescription( "outmax", "Maximum value of the output image." );
    MandatoryOff("outmin");
    MandatoryOff("outmax");

    AddRAMParameter();

    // Doc example parameter settings
    SetDocExampleParameterValue("in", "QB_Toulouse_Ortho_XS.tif");
    SetDocExampleParameterValue("out", "otbConvertWithScalingOutput.png");
    SetDocExampleParameterValue("type", "linear");
    SetDocExampleParameterValue("channels", "rgb");
    SetDocExampleParameterValue("outmin", "0");
    SetDocExampleParameterValue("outmax", "255");

    SetOfficialDocLink();
  }

  void DoUpdateParameters() ITK_OVERRIDE
  {
    // Read information
    if ( HasValue("in") )
    {
      typedef otb::ImageMetadataInterfaceBase ImageMetadataInterfaceType;
      ImageMetadataInterfaceType::Pointer metadataInterface =
        ImageMetadataInterfaceFactory::CreateIMI(
          GetParameterImage("in")->GetMetaDataDictionary());

      int nbBand = GetParameterImage("in")->GetNumberOfComponentsPerPixel();
      SetMaximumParameterIntValue("channels.grayscale.channel", nbBand);
      SetMaximumParameterIntValue("channels.rgb.red", nbBand);
      SetMaximumParameterIntValue("channels.rgb.green", nbBand);
      SetMaximumParameterIntValue("channels.rgb.blue", nbBand);

      if (nbBand > 1)
      {
        // get band index : Red/Green/Blue, in depending on the sensor
        auto const& display = metadataInterface->GetDefaultDisplay();
        SetDefaultParameterInt("channels.rgb.red", display[0] + 1);
        SetDefaultParameterInt("channels.rgb.green", display[1] + 1);
        SetDefaultParameterInt("channels.rgb.blue", display[2] + 1);
      }
    }
  }

  template<class TImageType>
  void GenericDoExecute()
  {
    // Clear previously registered filters
    m_Filters.clear();

    std::string rescaleType = this->GetParameterString("type");
    typedef otb::VectorRescaleIntensityImageFilter<FloatVectorImageType, TImageType> RescalerType;
    typename RescalerType::Pointer rescaler = RescalerType::New();

    // selected channel
    auto tempImage = GetSelectedChannels<FloatVectorImageType>();

    const unsigned int nbComp(tempImage->GetNumberOfComponentsPerPixel());

    // We need to subsample the input image in order to estimate its histogram
    // Shrink factor is computed so as to load a quicklook of 1000
    // pixels square at most
    auto imageSize = tempImage->GetLargestPossibleRegion().GetSize();
    unsigned int shrinkFactor = std::max({int(imageSize[0])/1000, 
      int(imageSize[1])/1000, 1});
    otbAppLogDEBUG( << "Shrink factor used to compute Min/Max: "<<shrinkFactor );

    otbAppLogDEBUG( << "Shrink starts..." );
    typename ShrinkFilterType::Pointer shrinkFilter = ShrinkFilterType::New();
    shrinkFilter->SetShrinkFactor(shrinkFactor);
    shrinkFilter->GetStreamer()->
      SetAutomaticAdaptativeStreaming(GetParameterInt("ram"));
    AddProcess(shrinkFilter->GetStreamer(), 
      "Computing shrink Image for min/max estimation...");

    if ( rescaleType == "log2")
    {
      //define the transfer log
      m_TransferLog = TransferLogType::New();
      m_TransferLog->SetInput(tempImage);
      m_TransferLog->UpdateOutputInformation();

      shrinkFilter->SetInput(m_TransferLog->GetOutput());
      rescaler->SetInput(m_TransferLog->GetOutput());
      shrinkFilter->Update();
    }
    else
    {
      shrinkFilter->SetInput(tempImage);
      rescaler->SetInput(tempImage);
      shrinkFilter->Update();
    }

    otbAppLogDEBUG( << "Evaluating input Min/Max..." );
    itk::ImageRegionConstIterator<FloatVectorImageType>
      it(shrinkFilter->GetOutput(), 
        shrinkFilter->GetOutput()->GetLargestPossibleRegion());

    typename ListSampleType::Pointer listSample = ListSampleType::New();
    listSample->SetMeasurementVectorSize( 
      tempImage->GetNumberOfComponentsPerPixel());

    // Now we generate the list of samples
    if (IsParameterEnabled("mask"))
    {
      FloatVectorImageType::Pointer mask = this->GetParameterImage("mask");
      ShrinkFilterType::Pointer maskShrinkFilter = ShrinkFilterType::New();
      maskShrinkFilter->SetShrinkFactor(shrinkFactor);
      maskShrinkFilter->SetInput(mask);
      maskShrinkFilter->GetStreamer()->
        SetAutomaticAdaptativeStreaming(GetParameterInt("ram"));
      maskShrinkFilter->Update();

      auto itMask = itk::ImageRegionConstIterator<FloatVectorImageType>(
        maskShrinkFilter->GetOutput(),
        maskShrinkFilter->GetOutput()->GetLargestPossibleRegion());

      // Remove masked pixels
      it.GoToBegin();
      itMask.GoToBegin();
      for(; !it.IsAtEnd(); ++it, ++itMask)
      {
        // float values, so the threshold is set to 0.5
        if (itMask.Get()[0] > 0.5)
        {
          listSample->PushBack(it.Get());
        }
      }
      // if listSample is empty
      if (listSample->Size() == 0)
      {
        otbAppLogINFO( << "All pixels were masked, the application assume "
          "a wrong mask and include all the image");
      }
    }

    // get all pixels : if mask is disable or all pixels were masked
    if ((!IsParameterEnabled("mask")) || (listSample->Size() == 0))
    {
      for(it.GoToBegin(); !it.IsAtEnd(); ++it)
      {
        listSample->PushBack(it.Get());
      }
    }

    // And then the histogram
    typename HistogramsGeneratorType::Pointer histogramsGenerator = 
      HistogramsGeneratorType::New();
    histogramsGenerator->SetListSample(listSample);
    histogramsGenerator->SetNumberOfBins(255);
    // Samples with nodata values are ignored
    histogramsGenerator->NoDataFlagOn();
    histogramsGenerator->Update();
    auto histOutput = histogramsGenerator->GetOutput();
    assert(histOutput);

    // And extract the lower and upper quantile
    typename FloatVectorImageType::PixelType inputMin(nbComp), inputMax(nbComp);
    for(unsigned int i = 0; i < nbComp; ++i)
    {
      auto && elm = histOutput->GetNthElement(i);
      assert(elm);
      inputMin[i] = elm->Quantile(0, 
        0.01 * GetParameterFloat("quantile.low"));
      inputMax[i] = elm->Quantile(0, 
        1.0 - 0.01 * GetParameterFloat("quantile.high"));
    }

    otbAppLogDEBUG( << std::setprecision(5) 
                    << "Min/Max computation done : min=" 
                    << inputMin
                    << " max=" << inputMax );

    rescaler->AutomaticInputMinMaxComputationOff();
    rescaler->SetInputMinimum(inputMin);
    rescaler->SetInputMaximum(inputMax);

    if ( rescaleType == "linear")
    {
      rescaler->SetGamma(GetParameterFloat("type.linear.gamma"));
    }

    typename TImageType::PixelType minimum(nbComp);
    typename TImageType::PixelType maximum(nbComp);

    /*
    float outminvalue = std::numeric_limits<typename TImageType::InternalPixelType>::min();
    float outmaxvalue = std::numeric_limits<typename TImageType::InternalPixelType>::max();
    // TODO test outmin/outmax values
    if (outminvalue > GetParameterFloat("outmin"))
      itkExceptionMacro("The outmin value at " << GetParameterFloat("outmin") << 
                        " is too low, select a value in "<< outminvalue <<" min.");
    if ( outmaxvalue < GetParameterFloat("outmax") )
      itkExceptionMacro("The outmax value at " << GetParameterFloat("outmax") << 
                        " is too high, select a value in "<< outmaxvalue <<" max.");
    */

    maximum.Fill( GetParameterFloat("outmax") );
    minimum.Fill( GetParameterFloat("outmin") );

    rescaler->SetOutputMinimum(minimum);
    rescaler->SetOutputMaximum(maximum);

    m_Filters.push_back(rescaler.GetPointer());
    SetParameterOutputImage<TImageType>("out", rescaler->GetOutput());
  }

  // Get the bands order
  std::vector<int> const GetChannels()
  {
    std::vector<int> channels;

    int nbChan = GetParameterImage("in")->GetNumberOfComponentsPerPixel();
    std::string channelMode = GetParameterString("channels");

    if(channelMode == "grayscale")
    {
      if (GetParameterInt("channels.grayscale.channel") <= nbChan)
      {
        channels = {GetParameterInt("channels.grayscale.channel"),
        GetParameterInt("channels.grayscale.channel"),
        GetParameterInt("channels.grayscale.channel")};
      }
      else
      {
        itkExceptionMacro(<< "The channel has an invalid index");
      }
    }
    else if (channelMode == "rgb")
    {
      if ((GetParameterInt("channels.rgb.red") <= nbChan)
        && ( GetParameterInt("channels.rgb.green") <= nbChan)
        && ( GetParameterInt("channels.rgb.blue")   <= nbChan))
      {
        channels = {GetParameterInt("channels.rgb.red"),
        GetParameterInt("channels.rgb.green"),
        GetParameterInt("channels.rgb.blue")};
      }
      else
      {
        itkExceptionMacro(<< "At least one needed channel has an invalid "
          "index");
      }
    }
    else if (channelMode == "all")
    {
      // take all bands
      channels.resize(nbChan);
      std::iota(channels.begin(), channels.end(), 1);
    }
    return channels;
  }

  // return an image with the bands order modified of the input image
  template<class TImageType>
  typename TImageType::Pointer GetSelectedChannels()
  {
    typedef MultiToMonoChannelExtractROI<FloatVectorImageType::InternalPixelType,
      typename TImageType::InternalPixelType> ExtractROIFilterType;
    typedef otb::ImageList<otb::Image<typename TImageType::InternalPixelType> > ImageListType;
    typedef ImageListToVectorImageFilter<ImageListType,
                                         TImageType > ListConcatenerFilterType;

    typename ImageListType::Pointer imageList  = ImageListType::New();
    typename ListConcatenerFilterType::Pointer concatener = 
      ListConcatenerFilterType::New();

    //m_Filters.push_back(imageList.GetPointer());
    m_Filters.push_back(concatener.GetPointer());

    const bool monoChannel = IsParameterEnabled("channels.grayscale");

    // get band order
    const std::vector<int> channels = GetChannels();

    for (auto && channel : channels)
    {
      typename ExtractROIFilterType::Pointer extractROIFilter = 
        ExtractROIFilterType::New();
      m_Filters.push_back(extractROIFilter.GetPointer());
      extractROIFilter->SetInput(GetParameterImage("in"));
      if (!monoChannel) 
        extractROIFilter->SetChannel(channel);

      extractROIFilter->UpdateOutputInformation();
      imageList->PushBack(extractROIFilter->GetOutput());
    }

    concatener->SetInput(imageList);
    concatener->UpdateOutputInformation();

    return concatener->GetOutput();
  }


  void DoExecute() override
  {
    switch ( this->GetParameterOutputImagePixelType("out") )
    {
      case ImagePixelType_uint8:
        GenericDoExecute<UInt8VectorImageType>();
        break;
      case ImagePixelType_int16:
        GenericDoExecute<Int16VectorImageType>();
        break;
      case ImagePixelType_uint16:
        GenericDoExecute<UInt16VectorImageType>();
        break;
      case ImagePixelType_int32:
        GenericDoExecute<Int32VectorImageType>();
        break;
      case ImagePixelType_uint32:
        GenericDoExecute<UInt32VectorImageType>();
        break;
      case ImagePixelType_float:
        GenericDoExecute<FloatVectorImageType>();
        break;
      case ImagePixelType_double:
        GenericDoExecute<DoubleVectorImageType>();
        break;
      default:
        itkExceptionMacro("Unknown pixel type "
          <<this->GetParameterOutputImagePixelType("out")<<".");
        break;
    }
  }

  itk::ProcessObject::Pointer m_TmpFilter;
  TransferLogType::Pointer m_TransferLog;
  std::vector<itk::LightObject::Pointer> m_Filters;
};

}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::DynamicConvert)

