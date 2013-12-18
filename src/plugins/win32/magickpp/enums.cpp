#include "magickpp.hpp"

using namespace Magick;

// Image class types
MAGICK_ENUM(ClassType) {
	ENUM(UndefinedClass);
	ENUM(DirectClass);
	ENUM(PseudoClass);
}

// Channel types
MAGICK_ENUM(ChannelType) {
	ENUM(UndefinedChannel);
	ENUM(RedChannel);
	ENUM(CyanChannel);
	ENUM(GreenChannel);
	ENUM(MagentaChannel);
	ENUM(BlueChannel);
	ENUM(YellowChannel);
	ENUM(OpacityChannel);
	ENUM(BlackChannel);
	ENUM(MatteChannel);
	ENUM(DefaultChannels);
}

// Color-space types
MAGICK_ENUM(ColorspaceType) {
	ENUM(CMYKColorspace);
	ENUM(GRAYColorspace);
	ENUM(HSLColorspace);
	ENUM(HWBColorspace);
	ENUM(LABColorspace);
	ENUM(LogColorspace);
	ENUM(OHTAColorspace);
	ENUM(Rec601LumaColorspace);
	ENUM(Rec709LumaColorspace);
	ENUM(RGBColorspace);
	ENUM(sRGBColorspace);
	ENUM(TransparentColorspace);
	ENUM(UndefinedColorspace);
	ENUM(XYZColorspace);
	ENUM(YCbCrColorspace);
	ENUM(YCCColorspace);
	ENUM(YIQColorspace);
	ENUM(YPbPrColorspace);
	ENUM(YUVColorspace);
}

// Composition operations
MAGICK_ENUM(CompositeOperator) {
	ENUM(AddCompositeOp);
	ENUM(AtopCompositeOp);
	ENUM(BlendCompositeOp);
	ENUM(BumpmapCompositeOp);
	ENUM(ClearCompositeOp);
	ENUM(ColorizeCompositeOp);
	ENUM(CopyBlueCompositeOp);
	ENUM(CopyCompositeOp);
	ENUM(CopyCyanCompositeOp);
	ENUM(CopyGreenCompositeOp);
	ENUM(CopyMagentaCompositeOp);
	ENUM(CopyOpacityCompositeOp);
	ENUM(CopyRedCompositeOp);
	ENUM(CopyYellowCompositeOp);
	ENUM(DarkenCompositeOp);
	ENUM(DifferenceCompositeOp);
	ENUM(DisplaceCompositeOp);
	ENUM(DissolveCompositeOp);
	ENUM(DstOverCompositeOp);
	ENUM(ExclusionCompositeOp);
	ENUM(HardLightCompositeOp);
	ENUM(HueCompositeOp);
	ENUM(InCompositeOp);
	ENUM(LightenCompositeOp);
	ENUM(LuminizeCompositeOp);
	ENUM(MinusCompositeOp);
	ENUM(ModulateCompositeOp);
	ENUM(MultiplyCompositeOp);
	ENUM(NoCompositeOp);
	ENUM(OutCompositeOp);
	ENUM(OverCompositeOp);
	ENUM(OverlayCompositeOp);
	ENUM(PlusCompositeOp);
	ENUM(SaturateCompositeOp);
	ENUM(ScreenCompositeOp);
	ENUM(SoftLightCompositeOp);
	ENUM(SubtractCompositeOp);
	ENUM(ThresholdCompositeOp);
	ENUM(UndefinedCompositeOp);
	ENUM(XorCompositeOp);
	ENUM(CopyBlackCompositeOp);
}

// Compression algorithms
MAGICK_ENUM(CompressionType) {
	ENUM(UndefinedCompression);
	ENUM(NoCompression);
	ENUM(BZipCompression);
	ENUM(FaxCompression);
	ENUM(Group4Compression);
	ENUM(JPEGCompression);
	ENUM(LZWCompression);
	ENUM(RLECompression);
	ENUM(ZipCompression);
}

MAGICK_ENUM(DisposeType) {
	ENUM(UndefinedDispose);
	ENUM(NoneDispose);
	ENUM(BackgroundDispose);
	ENUM(PreviousDispose);
}

// Endian options
MAGICK_ENUM(EndianType) {
	ENUM(UndefinedEndian);
	ENUM(LSBEndian);
	ENUM(MSBEndian);
}

// Evaluate options
MAGICK_ENUM(MagickEvaluateOperator) {
	ENUM(UndefinedEvaluateOperator);
	ENUM(AddEvaluateOperator);
	ENUM(AndEvaluateOperator);
	ENUM(DivideEvaluateOperator);
	ENUM(LeftShiftEvaluateOperator);
	ENUM(MaxEvaluateOperator);
	ENUM(MinEvaluateOperator);
	ENUM(MultiplyEvaluateOperator);
	ENUM(OrEvaluateOperator);
	ENUM(RightShiftEvaluateOperator);
	ENUM(SetEvaluateOperator);
	ENUM(SubtractEvaluateOperator);
	ENUM(XorEvaluateOperator);
}

// Fill rules
MAGICK_ENUM(FillRule) {
	ENUM(UndefinedRule);
	ENUM(EvenOddRule);
	ENUM(NonZeroRule);
}

// Filter types
MAGICK_ENUM(FilterTypes) {
	ENUM(UndefinedFilter);
	ENUM(PointFilter);
	ENUM(BoxFilter);
	ENUM(TriangleFilter);
	ENUM(HermiteFilter);
	ENUM(HanningFilter);
	ENUM(HammingFilter);
	ENUM(BlackmanFilter);
	ENUM(GaussianFilter);
	ENUM(QuadraticFilter);
	ENUM(CubicFilter);
	ENUM(CatromFilter);
	ENUM(MitchellFilter);
	ENUM(LanczosFilter);
	ENUM(BesselFilter);
	ENUM(SincFilter);
}

// Bit gravity
MAGICK_ENUM(GravityType) {
	ENUM(ForgetGravity);
	ENUM(NorthWestGravity);
	ENUM(NorthGravity);
	ENUM(NorthEastGravity);
	ENUM(WestGravity);
	ENUM(CenterGravity);
	ENUM(EastGravity);
	ENUM(SouthWestGravity);
	ENUM(SouthGravity);
	ENUM(SouthEastGravity);
	ENUM(StaticGravity);
}

// Image types
MAGICK_ENUM(ImageType) {
	ENUM(UndefinedType);
	ENUM(BilevelType);
	ENUM(GrayscaleType);
	ENUM(GrayscaleMatteType);
	ENUM(PaletteType);
	ENUM(PaletteMatteType);
	ENUM(TrueColorType);
	ENUM(TrueColorMatteType);
	ENUM(ColorSeparationType);
	ENUM(ColorSeparationMatteType);
	ENUM(OptimizeType);
}

// Interlace types
MAGICK_ENUM(InterlaceType) {
	ENUM(UndefinedInterlace);
	ENUM(NoInterlace);
	ENUM(LineInterlace);
	ENUM(PlaneInterlace);
	ENUM(PartitionInterlace);
}

// Line cap types
MAGICK_ENUM(LineCap) {
	ENUM(UndefinedCap);
	ENUM(ButtCap);
	ENUM(RoundCap);
	ENUM(SquareCap);
}

// Line join types
MAGICK_ENUM(LineJoin) {
	ENUM(UndefinedJoin);
	ENUM(MiterJoin);
	ENUM(RoundJoin);
	ENUM(BevelJoin);
}

// Noise types
MAGICK_ENUM(NoiseType) {
	ENUM(UniformNoise);
	ENUM(GaussianNoise);
	ENUM(MultiplicativeGaussianNoise);
	ENUM(ImpulseNoise);
	ENUM(LaplacianNoise);
	ENUM(PoissonNoise);
}

// Orientation types
MAGICK_ENUM(OrientationType) {
	ENUM(UndefinedOrientation);
	ENUM(TopLeftOrientation);
	ENUM(TopRightOrientation);
	ENUM(BottomRightOrientation);
	ENUM(BottomLeftOrientation);
	ENUM(LeftTopOrientation);
	ENUM(RightTopOrientation);
	ENUM(RightBottomOrientation);
	ENUM(LeftBottomOrientation);
}

// Paint methods
MAGICK_ENUM(PaintMethod) {
	ENUM(PointMethod);
	ENUM(ReplaceMethod);
	ENUM(FloodfillMethod);
	ENUM(FillToBorderMethod);
	ENUM(ResetMethod);
}

// Quantum types
MAGICK_ENUM(QuantumType) {
	ENUM(IndexQuantum);
	ENUM(GrayQuantum);
	ENUM(IndexAlphaQuantum);
	ENUM(GrayAlphaQuantum);
	ENUM(RedQuantum);
	ENUM(CyanQuantum);
	ENUM(GreenQuantum);
	ENUM(YellowQuantum);
	ENUM(BlueQuantum);
	ENUM(MagentaQuantum);
	ENUM(AlphaQuantum);
	ENUM(BlackQuantum);
	ENUM(RGBQuantum);
	ENUM(RGBAQuantum);
	ENUM(CMYKQuantum);
}

// Rendering intents
MAGICK_ENUM(RenderingIntent) {
	ENUM(UndefinedIntent);
	ENUM(SaturationIntent);
	ENUM(PerceptualIntent);
	ENUM(AbsoluteIntent);
	ENUM(RelativeIntent);
}

// Resolution units
MAGICK_ENUM(ResolutionType) {
	ENUM(UndefinedResolution);
	ENUM(PixelsPerInchResolution);
	ENUM(PixelsPerCentimeterResolution);
}

// StorageType type
MAGICK_ENUM(StorageType) {
	ENUM(CharPixel);
	ENUM(ShortPixel);
	ENUM(IntegerPixel);
	ENUM(FloatPixel);
	ENUM(DoublePixel);
}

// StretchType type
MAGICK_ENUM(StretchType) {
	ENUM(NormalStretch);
	ENUM(UltraCondensedStretch);
	ENUM(ExtraCondensedStretch);
	ENUM(CondensedStretch);
	ENUM(SemiCondensedStretch);
	ENUM(SemiExpandedStretch);
	ENUM(ExpandedStretch);
	ENUM(ExtraExpandedStretch);
	ENUM(UltraExpandedStretch);
	ENUM(AnyStretch);
}

// StyleType type
MAGICK_ENUM(StyleType) {
	ENUM(NormalStyle);
	ENUM(ItalicStyle);
	ENUM(ObliqueStyle);
	ENUM(AnyStyle);
}

// Decoration types
MAGICK_ENUM(DecorationType) {
	ENUM(NoDecoration);
	ENUM(UnderlineDecoration);
	ENUM(OverlineDecoration);
	ENUM(LineThroughDecoration);
}

