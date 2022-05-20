// ITK includes
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkShapeLabelObject.h>
#include <itkExtractImageFilter.h>

// TCLAP includes
#include <tclap/ValueArg.h>
#include <tclap/ArgException.h>
#include <tclap/CmdLine.h>
#include <tclap/SwitchArg.h>

// STD includes
#include <cstdlib>

//NOTE: Here we assume the input and output data type is unsigned short!

int main (int argc, char **argv)
{

  // =========================================================================
  // Command-line variables
  // =========================================================================
  std::string inputImage;
  std::string outputImage;

  // =========================================================================
  // Parse arguments
  // =========================================================================
  try {

    TCLAP::CmdLine cmd("itkBoundingBox");

    TCLAP::ValueArg<std::string> input("i", "input", "Image to compute the output", true, "None", "string");
    TCLAP::ValueArg<std::string> output("o", "output", "Cropped imge", false, "None", "string");

    cmd.add(input);
    cmd.add(output);

    cmd.parse(argc,argv);

    inputImage = input.getValue();
    outputImage = output.getValue();

  } catch (TCLAP::ArgException &e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
  }

  // =========================================================================
  // ITK definitions
  // =========================================================================
  using ImageType = itk::Image<float, 3>;
  using ImageReaderType = itk::ImageFileReader<ImageType>;
  using ImageWriterType = itk::ImageFileWriter<ImageType>;
  using ShapeLabelObjectType = itk::ShapeLabelObject<ImageType::PixelType, ImageType::ImageDimension>;
  using LabelMapType = itk::LabelMap<ShapeLabelObjectType>;
  using LabelImageToShapeFilterType = itk::LabelImageToShapeLabelMapFilter<ImageType, LabelMapType>;
  using ExtractImageFilterType = itk::ExtractImageFilter<ImageType, ImageType>;

  // =========================================================================
  // Image loading and checking
  // =========================================================================
  auto inputReader = ImageReaderType::New();
  inputReader->SetFileName(inputImage);
  inputReader->Update();

  // =========================================================================
  // Compute statistics
  // =========================================================================
  auto labelImageToShapeFilter = LabelImageToShapeFilterType::New();
  labelImageToShapeFilter->SetInput(inputReader->GetOutput());
  labelImageToShapeFilter->Update();

  auto labelMap = labelImageToShapeFilter->GetOutput();
  auto labelObject = labelMap->GetNthLabelObject(1);
  std::cout << "BoundingBox: " << labelObject->GetBoundingBox() << std::endl;

  // =========================================================================
  // Extract the sub-volume
  // =========================================================================
  auto extractImageFilter = ExtractImageFilterType::New();
  extractImageFilter->SetInput(inputReader->GetOutput());
  extractImageFilter->SetExtractionRegion(labelObject->GetBoundingBox());
  extractImageFilter->Update();

  // =========================================================================
  // Write the sub-volume
  // =========================================================================
  if (outputImage != "None")
  {
    auto outputWriter = ImageWriterType::New();
    outputWriter->SetFileName(outputImage);
    outputWriter->SetInput(extractImageFilter->GetOutput());
    outputWriter->Write();
  }

  return EXIT_SUCCESS;
}
