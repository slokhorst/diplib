# Changes from DIPlib 2 {#changes}

[//]: # (DIPlib 3.0)

[//]: # ([c]2016-2019, Cris Luengo.)
[//]: # (Based on original DIPlib code: [c]1995-2014, Delft University of Technology.)
[//]: # (Based on original DIPimage code: [c]1999-2014, Delft University of Technology.)

[//]: # (Licensed under the Apache License, Version 2.0 [the "License"];)
[//]: # (you may not use this file except in compliance with the License.)
[//]: # (You may obtain a copy of the License at)
[//]: # ()
[//]: # (   http://www.apache.org/licenses/LICENSE-2.0)
[//]: # ()
[//]: # (Unless required by applicable law or agreed to in writing, software)
[//]: # (distributed under the License is distributed on an "AS IS" BASIS,)
[//]: # (WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.)
[//]: # (See the License for the specific language governing permissions and)
[//]: # (limitations under the License.)

*DIPlib 3* is a complete rewrite in C++ of the *DIPlib 2* infrastructure, which was written
in C; only the code that implements actual image processing and analysis algorithms is ported
over.

The list below describes infrastructure changes, function interface changes, and functionality
changes in the *DIPlib* library, as well as changes to the *DIPimage* toolbox. It is possible
that some changes were not listed here, but hopefully this list will help in porting your old
code that used *DIPlib* or *DIPimage* to the new version.

\tableofcontents

[//]: # (--------------------------------------------------------------)

\section changes_core Core/infrastructure changes

- Functions and types used to start with `dip_`, now they are in the `dip::` namespace.

- Functions no longer return a `dip_Error`, they throw an exception instead. The `DIPXJ`
  et al. macros are gone. So are the `DIP_FNR_DECLARE` / `DIP_FNR_INITIALISE` /
  `DIP_FNR_EXIT` macros.

- The array types are now based on either `std::vector` or `dip::DimensionArray`, which
  is a short-array optimized version of `std::vector` with a bunch of specialized algorithms
  as member functions. Consequently, all access functions are no longer, to be replaced by
  methods with different names and syntax.

- The core image class, `dip_Image`, now is `dip::Image`, with a much simplified interface.
  As with the array types, access functions are no longer, to be replaced by class methods.
  Many of these class methods have names similar to the original access functions, but not
  always. For example, `dip_ImageSetStride` now is `dip::Image::SetStrides`.

   - `dip_Assimilate` is the `dip::Image::ReForge` method.

   - We used to call the image size its dimension. `dip_Dimensions` is now `dip::Image::Sizes`.
     The reason is that it was too confusing talking about a dimension as an image axis (the
     2<sup>nd</sup> dimension), and the dimension of that dimension (the size of the image along that
     axis).

   - Images now can have a tensor as pixel value, as was possible in *DIPimage*. To port old
     functions, you can test for `dip::Image::IsScalar` and ignore this new feature, or
     modify the old function to work with tensor values.

   - Images now carry the physical dimension in them (referred to as pixel size). When porting
     functions, think about whether this data needs to be maintained, modified, or removed.

   - The same is true for the color space. A function that changes the number of tensor
     elements must also remove the color space information.

   - Please read the documentation to the `dip::Image` class before doing any work with the
     library!

- The framework functions all have different names:
    - `dip_SeparableFrameWork` &rarr; `dip::Framework::Separable`
    - `dip_MonadicFrameWork` &rarr; `dip::Framework::ScanMonadic` / `dip::Framework::SeparableMonadic`
    - `dip_MonadicPoint` &rarr; `dip::Framework::ScanMonadic`
    - `dip_MonadicPointData` &rarr; `dip::Framework::ScanMonadic`
    - `dip_PixelTableArrayFrameWork` &rarr; doesn't exist any more, was used meaningfully only in `dip_GeneralisedKuwaharaImproved`
    - `dip_PixelTableFrameWork` &rarr; `dip::Framework::Full`
    - `dip_ScanFrameWork` &rarr; `dip::Framework::Scan()` / `dip::Framework::ScanDyadic`
    - `dip_SingleOutputFrameWork` &rarr; `dip::Framework::ScanSingleOutput`
    - `dip_SingleOutputPoint` &rarr; `dip::Framework::ScanSingleOutput`

  Their interfaces are not exactly compatible, but it should be relatively straightforward
  to port old line functions to use the new framework, yielding shorter code.
  However, all line functions are now expected to use strides.

- `dip_ovl.h` is now `diplib/overload.h`, and works like a normal header file defining
  macros to use in your code. `dip_tpi.h` is gone, the template strategy to define overloaded
  functions is now based on C++ templates, yielding code that is easier to write and easier
  to read.

- There is no longer a `dip_Initialise` function. There are no registries.
  There is only one global variable: the number of threads to use with OpenMP (see
  `dip::GetNumberOfThreads` and `dip::SetNumberOfThreads`).

- There is no longer a `dip_ImagesSeparate` function. Its functionality can be accomplished
  using a simple image copy (the copy shares the image data with the original image, and
  makes it possible to strip the original image while still keeping the input data available).

- Header files used to have names such as `dip_xxx.h`, they now are `diplib/xxx.h`. The "`xxx`"
  part has remained the same in most cases, though some function declarations have moved to a
  different header file. The documentation specifies which header file to include for each
  function.

- The *dipIO* library no longer exists. Some `dipio_Xxx` functions are now defined in the
  `diplib/file_io.h` header file (reading and writing ICS, TIFF and JPEG files).
  `dipio_MeasurementWriteCSV` is is `diplib/measurement.h`.
  `dipio_Colour2Gray` functionality is replaced by `dip::ColorSpaceManager`.
  Other functionality no longer exists.

[//]: # (--------------------------------------------------------------)

\section changes_interface Changes in algorithm interface

- Function parameters expressing options are now represented by strings rather than
  \c \#define or `enum` constants (except for low-level library functionality).

- The library now makes a distinction between signed and unsigned integers. This affects many
  function parameters.

- Output arguments are now always on the left-hand side, except for output images.

- We try to sort the arguments most commonly left with default values at the end of the
  argument lists. This caused some functions to have a different parameter order.
  For example, the boundary condition is not commonly changed, and so the boundary condition
  array input argument is now typically further to the right in (usually at the end of) the
  argument list.

- Function names are often simplified, since in C++ it's possible to overload functions for
  different types. For example, `dip_BesselJ0` and `dipm_BesselJ0` are now both called
  `dip::BesselJ0`. `dip_BooleanArrayUseParameter`, `dip_IntegerArrayUseParameter`, etc. are
  now all `dip::ArrayUseParameter`.

- Some other function names that included only parts of a name or verb (they were shortened)
  have been renamed to use full names. For example: `dip_Sub` is now `dip::Subtract`;
  `dip_LaplaceMinDgg` is now called `dip::LaplaceMinusDgg`; `dip_RadialMin` and `dip_RadialMax`
  are now called `dip::RadialMinimum` and `dip::RadialMaximum`.

- `dip_Measure` is now `dip::MeasurementTool::Measure`, with `dip::MeasurementTool` an object
  that knows about defined measurement features.

   - Measurement features are registered with a `dip::MeasurementTool` object, not in the
     global registry.

   - Measurement features have a much simpler programmer's interface, it should be easier to
     write new features now.

- `dip_ObjectToMeasurement` now takes a feature iterator rather than computing the feature;
  this might be more flexible.

- `dip_Arith` and `dip_Compare` used to implement all arithmetic and comparison operators,
  (with convenience macros `dip_Add`, `dip_Sub`, etc.). These functions no longer exist,
  each operator is implemented by its own function. The shortened names of these functions
  are no longer shortened, so instead of `dip_Sub` use `dip::Subtract`. `dip_AddInteger`,
  `dip_AddFloat`, etc. now are overloaded versions of `dip::Add` et al.

- `dip_WeightedAdd` is now generalized to `dip::LinearCombination`.
  `dip_WeightedSub` can be emulated by setting a negative weight.
  `dip_WeightedMul` and `dip_WeightedDiv` were not used anywhere, and don't seem very useful.

- Morphological filters now use a `dip::StructuringElement` to collect three parameters
  of *DIPlib 2*. Other filters that had a filter shape argument now use a
  `dip::Kernel` to collect three parameters.

- `dip_Min` and `dip_Max` are now `dip::Infimum` and `dip::Supremum`, and `dip_SignedMinimum`
  is now `dip::SignedInfimum`. There are too many functions with "Maximum" and "Minimum" in
  their name.

- `dip_MultiDimensionalHistogram` and `dip_ImageToHistogram` have been merged into the
  constructor for `dip::Histogram`, which replaces `dip_Histogram`.

- `dip::Distribution` is a new class that replaces `dip_Distribution`. `dip_Distribution` was
  identical to `dip_Histogram`. `dip::Distribution` is very different from `dip::Histogram`.

- The `dip_ImageLookup` function has been merged with the `dip::LookupTable` object, and
  includes the functionality of `dip_ImageLut`.

- `dip_Threshold` is now `dip::FixedThreshold`. `dip::Threshold` is a function that allows
  to select an automated threshold method.

- `dip_RandomXxx` functions are now methods to the `dip::Random` class. `dip_UniformRandomVariable`
  and similar functions are now classes `dip::UniformRandomGenerator` and similar.
  `dip::GaussianRandomGenerator` produces a single output value, the object stores the
  second value for the next call.

- `dip_LabelSetBorder`, `dip_ImageDoEdge` and `dip_PlaneDoEdge` are merged into `dip::SetBorder`,
  which additionally can specify the border size.

- `dip_LocalMinima` is now called `dip::WatershedMinima`. There's also a maxima variant.

- `dip_GetMaximumAndMinimum` is now called `dip::MaximumAndMinimum` for consistency.

- `dip_ResamplingFT` is no longer a separate function, call `dip::Resampling` with "ft" as
  method. `dip_Skewing` was renamed to `dip::Skew` for consistency. `dip_Rotation` has
  been renamed `dip::Rotation2D`, and a generalized `dip::Rotation` is similar but takes
  two additional integers to specify the rotation plane. `dip_Rotation3d_Axis` and
  `dip_Rotation3d` are now both called `dip::Rotation3D`, their argument types are different.
  `dip::Resampling`, `dip::Skew`, `dip::Rotation` and similar now take a boundary condition
  as optional argument. `dip::Skew` has a new parameter that allows selection of the image
  line that has 0 shift. `dip::Rotation` now rotates around the pixel at the origin (this is
  the pixel to the right of the geometric center for even-sized images), rather than the geometric
  center of the image.

- `dip_Rotation2d90`, `dip_Rotation2d90`, `dip_Map`, `dip_Mirror`, and `dip_Crop` are now all
  member functions of `dip::Image`, and modify the existing image (without copying data).
  `dip_GetSlice`, `dip_PutSlice`, `dip_PutSubSpace`, `dip_GetLine`, `dip_PutLine`, `dip_Set`,
  `dip_SetXxx`, `dip_Get`, `dip_GetXxx` are now all implemented through indexing operations
  on the `dip::Image` object (see the many overloads of `dip::Image::At`) and assignment to
  the output of these operations.

- `dip_KuwaharaImproved` is now called `dip::Kuwahara`, and `dip_Kuwahara` is no longer
  (C++ default values make it redundant).

- `dip_GeneralisedKuwaharaImproved` is now called `dip::SelectionFilter`, and
  `dip_GeneralisedKuwahara` is no longer (C++ default values make it redundant).

- `dip_PaintBox`, `dip_PaintDiamond` and `dip_PaintEllipsoid` are now called `dip::DrawBox`,
  `dip::DrawDiamond` and `dip::DrawEllipsoid`, respectively (for consistency). `dip_DrawLineFloat` and
  `dip_DrawLineComplex` are now a single function `dip::DrawLine`; the same is true for `dip::DrawLines`.

- `dip_RadialDistribution` renamed to `dip::DistanceDistribution`.

- `dip_Bilateral` renamed to `dip::SeparableBilateralFilter`, and `dip_BilateralFilter`
  renamed to `dip::FullBilateralFilter`. Added new function `dip::BilateralFilter` as
  an interface to all bilateral filter variants.

- `dip_TestObjectCreate`, `dip_TestObjectModulate`, `dip_TestObjectBlur` and
  `dip_TestObjectAddNoise` have been merged into a single function: `dip::TestObject`.
  The new function additionally has the option to generate test objects with Gaussian
  edges directly in the spatial domain.

- `dip_GenerateRamp` is now `dip::CreateRamp`, and has convenience functions `dip::FillRamp`,
  `dip::FillXCoordinate`, `dip::CreateXCoordinate`, `dip::FillYCoordinate`, `dip::CreateYCoordinate`,
  `dip::FillZCoordinate`, `dip::CreateZCoordinate`.

- `dip::EuclideanDistanceToPoint` and friends now take the sizes of the output image to create. They
  are generalized in the functions `dip::DistanceToPoint` and `dip::FillDistanceToPoint`.

- `dip::ClipLow` and `dip::ClipHigh` are convenience interfaces to `dip::Clip`.

[//]: # (--------------------------------------------------------------)

\section changes_functionality Changes in functionality

- Second order extrapolation boundary extension didn't do as advertised in *DIPlib 2*.
  Also the first order extrapolation didn't work correctly with unsigned integers.
  The new implementation fits a 1<sup>st</sup> or 2<sup>nd</sup> order polynomial that reaches
  0 at the end of the extended boundary, with the 2<sup>nd</sup> order polynomial matching
  two samples at the image edge and therefore connects smoothly.
  A 3<sup>rd</sup> order extrapolation has been added, which works similarly to the
  2<sup>nd</sup> order extrapolation but reaches 0 smoothly at the end of the boundary.
  These functions are quite noise sensitive, however, and produce high frequencies along
  the edge.

- The mirror and asymmetric boundary extension methods have been fixed to work correctly
  when the boundary is larger than the image. Mirror no longer repeats the pixel at the
  edge of the image.

- Fixed little bug in the "Perimeter" measurement feature, which didn't check the corner count
  for the transition from the last to the first chain code, yielding a perimeter that was often
  0.0910 pixels too long. The feature "Radius" now computes the center of mass of the object
  correctly.

- The measurement features "Skewness" and "ExcessKurtosis" are no longer separate features, but
  instead put together in the new "Statistics" feature, which also computes the mean and standard
  deviation. The measurement feature "StdDev" is now called "StandardDeviation".
  The algorithms to compute these statistics have changed to be more stable. The measurement
  feature "Sum" was an alias to "Mass", and is not (yet?) available.

- The measurement feature "Convexity" has been renamed to "Solidity", the more common name
  for the ratio of area to convex area.

- New measurement features: "SolidArea", "Circularity", "Convexity", "Eccentricity", "EllipseVariance",
  "Roundness", "DimensionsCube", "DimensionsEllipsoid", "GreyDimensionsCube" and "GreyDimensionsEllipsoid".
  Note that "Convexity" therefore is not the same as what it was in the previous version of the library.

- The measurement features "Mu", "Inertia", "MajorAxes", and their grey-value versions have been generalized
  to arbitrary number of dimensions.

- `dip::SeparableConvolution` treats input filter definitions slightly differently, and there
  no longer are "left" and "right" options.

- `dip::ImageDisplay` no longer does any spatial scaling. Also, it's a class, not a function, and provides
  much more functionality than the function of the same name in the old version of the library.

- `dip::ColorSpaceManager` is functionality ported from MATLAB-code in *DIPimage 2*,
  with a few new color spaces added.

- `dip::FourierTransform` now does normalization in the more common way (forward transform not
  normalized, inverse transform normalized by 1/N), but an option ("symmetric") allows to change
  the normalization to be consistent with *DIPlib 2*, which used a symmetric normalization
  scheme (both forward and backward transforms use 1/N<sup>1/2</sup>)

- `dip::Histogram` misses a few of the options that `dip_MultiDimensionalHistogram` had, but I
  don't think they are relevant. They can be added easily if necessary.

- Many threshold selection strategies have been ported from MATLAB code in *DIPimage*, see
  `dip::Threshold`.

- `dip::Random` uses a different pseudo-random number generator. Previous versions used the
  Mersenne Twister. We now use the PCG scheme (permuted linear congruential generator), which
  is much faster than the Mersenne Twister, has a much smaller internal state, and produces
  better quality randomness, though with the given parameters, the period is not as large as
  that of the Mersenne Twister (2<sup>128</sup> vs 2<sup>19937</sup>, but do note that
  2<sup>128</sup> is a very, very long period).

- Morphological filters define line structuring elements differently than before. They used to
  be available only for 2D images, now they are generalized to nD. The filter parameter, instead
  of being a length and an angle, now represents the bounding box, with direction encoded by the
  signs. Besides the discrete line and the interpolated line, we now have several other options,
  including the periodic line, which makes translation-invariant operations with line SEs much
  more efficient. The "diamond" SE is now implemented using line SEs, and we have added an
  "octagonal" SE that is computed as a combination of a diamond SE and a rectangular SE.

- `dip::Rotation` and dependent functions now rotate around the pixel that is right from center
  (the same pixel that is assumed the origin in `dip::FourierTransform` and other functions),
  instead of the point in between pixels, in case of an even-sized image. The implementation is
  slightly more complex, but this definition is more useful. This function now works for any
  number of dimensions, though it only rotates around one cartesian axis. `dip::Rotation3D` is
  a new function that does a full 3D rotation.

- `dip::Skew` can now skew in multiple dimensions at the same time.

- `dip::Shift` now calls `dip::Resampling`, shifting the image in the spatial domain. `dip::ShiftFT`
  uses the frequency domain, as `dip_Shift` did.

- `dip::Resampling` (and by extension `dip::Shift`) shifts the image in the opposite direction
  from what it did in *DIPlib 2*, where the shift was unintuitive.

- `dip::FindShift` now returns the shift with inverse sign compared to before, to match the reversal
  in `dip::Shift`.

- `dip::ImageWriteICS` now has a `"fast"` option that causes dimensions to be written to file in
  the order in which they are stored in memory, rather than in standard increasing order. This
  makes writing a lot faster if strides are non-standard. `dip::ImageReadICS` has a similar option
  that re-orders strides for the output image to match those of the file, again potentially decreasing
  reading times significantly.

- `dip::Canny` and `dip::MaximumSuppression` have been generalized from 2D only to any
   number of dimensions.

- `dip::GreyWeightedDistanceTransform` now works for images of any dimensionality, and no longer
  excludes the pixels at the edge of the image. It also accepts an optional mask image. The Euclidean
  distance output argument has been merged with the grey-weighted distance output, a flag is used to
  choose which of these two outputs should be given. By default, it now uses the fast marching algorithm,
  which produces more isotropic results than the old algorithm.

- `dip::GrowRegions` no longer takes a grey-value image as input. Use `dip::SeededWatershed` instead.

- `dip::EuclideanDistanceTransform` has a new algorithm which produces exact distances in nD, and
  is parallelized and very fast. This new algorithm is the default.

- `dip::ResampleAt` has various signatures and a lot more flexibility than in the old library. There
  is also a `dip::ResampleAtUnchecked`, to be used together with `dip::PrepareResampleAtUnchecked`,
  for low-level use.

[//]: # (--------------------------------------------------------------)

\section new_functionality New functionality

- New external library DIPviewer adds interactive image display, see \ref viewer.

- New external library DIPjavaio adds the option to use *Bio-Formats* to read hundreds of image file formats,
  see \ref javaio.

- New analysis functions: `dip::MeanShift`, `dip::FourierMellinMatch2D`, `dip::MonogenicSignal`,
  `dip::MonogenicSignalAnalysis`, `dip::Semivariogram`, `dip::Granulometry`, `dip::FractalDimension`.

- New detection functions: `dip::HoughTransformCircleCenters`, `dip::FindHoughMaxima`, `dip::PointDistanceDistribution`,
  `dip::FindHoughCircles`, `dip::RadonTransformCircles`, `dip::HarrisCornerDetector`, `dip::ShiTomasiCornerDetector`,
  `dip::NobleCornerDetector`, `dip::WangBradyCornerDetector`, `dip::FrangiVesselness`,
  `dip::MatchedFiltersLineDetector2D`, `dip::DanielssonLineDetector`, `dip::RORPOLineDetector`.

- New distance function: `dip::GeodesicDistanceTransform`.

- New binary filtering functions: `dip::FillHoles`, `dip::ConditionalThickening2D`, `dip::ConditionalThinning2D`,
  `dip::BinaryAreaOpening`, `dip::BinaryAreaClosing`, `dip::CountNeighbors`, `dip::MajorityVote`, `dip::GetSinglePixels`,
  `dip::GetEndPixels`, `dip::GetLinkPixels`, `dip::GetBranchPixels`, `dip::SupGenerating`, `dip::InfGenerating`,
  `dip::UnionSupGenerating`, `dip::UnionSupGenerating2D`, `dip::IntersectionInfGenerating`,
  `dip::IntersectionInfGenerating2D`, `dip::Thickening`, `dip::Thickening2D`, `dip::Thinning`, `dip::Thinning2D`,
  `dip::HomotopicThinningInterval2D`, `dip::HomotopicThickeningInterval2D`, `dip::EndPixelInterval2D`,
  `dip::HomotopicEndPixelInterval2D`, `dip::HomotopicInverseEndPixelInterval2D`, `dip::SinglePixelInterval`,
  `dip::BranchPixelInterval2D`, `dip::BoundaryPixelInterval2D`, `dip::ConvexHullInterval2D`.

- New linear filtering functions: `dip::SeparateFilter`, `dip::Dx`, `dip::Dy`, `dip::Dz`, `dip::Dxx`, `dip::Dyy`,
  `dip::Dzz`, `dip::Dxy`, `dip::Dxz`, `dip::Dyz`, `dip::Gradient`, `dip::GradientDirection`, `dip::Curl`,
  `dip::Divergence`, `dip::Hessian`, `dip::UnsharpMask`, `dip::GaborFIR`, `dip::Gabor2D`, `dip::LogGaborFilterBank`,
  `dip::NormalizedConvolution`, `dip::NormalizedDifferentialConvolution`, `dip::MeanShiftVector`.

- New morphological functions: `dip::RankFilter`, `dip::RankMinClosing`, `dip::RankMaxOpening`,
  `dip::LimitedMorphologicalReconstruction`, `dip::HMaxima`, `dip::HMinima`, `dip::Leveling`, `dip::AreaClosing`,
  `dip::OpeningByReconstruction`, `dip::ClosingByReconstruction`, `dip::AlternatingSequentialFilter`,
  `dip::HitAndMiss`.

- New nonlinear filtering functions: `dip::PeronaMalikDiffusion`, `dip::GaussianAnisotropicDiffusion`,
  `dip::RobustAnisotropicDiffusion`, `dip::CoherenceEnhancingDiffusion`.

- New generation functions: `dip::FillRadiusCoordinate`, `dip::CreateRadiusCoordinate`,
  `dip::FillRadiusSquareCoordinate`, `dip::CreateRadiusSquareCoordinate`, `dip::FillPhiCoordinate`,
  `dip::CreatePhiCoordinate`, `dip::FillThetaCoordinate`, `dip::CreateThetaCoordinate`, `dip::FillCoordinates`,
  `dip::CreateCoordinates`,
  `dip::ApplyWindow`,
  `dip::DrawPolygon2D`, `dip::DrawBandlimitedPoint`, `dip::DrawBandlimitedLine`, `dip::DrawBandlimitedBall`,
  `dip::DrawBandlimitedBox`,
  `dip::SaltPepperNoise`, `dip::FillColoredNoise`, `dip::ColoredNoise`,
  `dip::GaussianEdgeClip`, `dip::GaussianLineClip`,
  `dip::FillDelta`, `dip::CreateDelta`, `dip::CreateGauss`, `dip::CreateGabor`,
  `dip::FillPoissonPointProcess`, `dip::FillRandomGrid`.

- New geometric transformation functions: `dip::RotationMatrix2D`, `dip::RotationMatrix3D`, `dip::AffineTransform`,
  `dip::LogPolarTransform2D`, `dip::Tile`, `dip::TileTensorElements`, `dip::Concatenate`, `dip::JoinChannels`.

- New grey-value mapping functions: `dip::Zero`, `dip::HistogramEqualization`, `dip::HistogramMatching`.

- New histogram functions: `dip::CumulativeHistogram`, `dip::Smooth`, `dip::Mean`, `dip::Covariance`,
  `dip::MarginalMedian`, `dip::Mode`, `dip::MutualInformation`, `dip::Entropy`, `dip::IsodataThreshold`,
  `dip::OtsuThreshold`, `dip::MinimumErrorThreshold`, `dip::TriangleThreshold`, `dip::BackgroundThreshold`,
  `dip::KMeansClustering`, `dip::MinimumVariancePartitioning`, `dip::EqualizationLookupTable`,
  `dip::MatchingLookupTable`, `dip::PerObjectHistogram`.

- New labeled regions function: `dip::Relabel`.

- New math and statistics functions: `dip::IsNotANumber`, `dip::IsInfinite`, `dip::IsFinite`, `dip::InRange`,
  `dip::OutOfRange`,
  `dip::MaximumAbsoluteError`, `dip::PSNR`, `dip::SSIM`, `dip::MutualInformation`,
  `dip::SpatialOverlap`, `dip::DiceCoefficient`, `dip::JaccardIndex`, `dip::Specificity`, `dip::Sensitivity`,
  `dip::Accuracy`, `dip::Precision`, `dip::HausdorffDistance`,
  `dip::Entropy`, `dip::EstimateNoiseVariance`,
  `dip::GeometricMean`, `dip::MeanSquare`, `dip::SumSquare`, `dip::MaximumAbs`, `dip::MinimumAbs`,
  `dip::MedianAbsoluteDeviation`, `dip::All`, `dip::Any`,
  `dip::Count`, `dip::CumulativeSum`, `dip::SampleStatistics`, `dip::Covariance`,  `dip::PearsonCorrelation`,
  `dip::SpearmanRankCorrelation`, `dip::CenterOfMass`, `dip::Moments`,
  `dip::Transpose`, `dip::ConjugateTranspose`, `dip::DotProduct`, `dip::CrossProduct`,
  `dip::Norm`, `dip::SquareNorm`, `dip::Angle`, `dip::Orientation`, `dip::CartesianToPolar`, `dip::PolarToCartesian`,
  `dip::Determinant`, `dip::Trace`, `dip::Rank`, `dip::Eigenvalues`, `dip::LargestEigenvalue`, `dip::SmallestEigenvalue`,
  `dip::EigenDecomposition`, `dip::LargestEigenvector`, `dip::SmallestEigenvector`, `dip::Inverse`, `dip::PseudoInverse`,
  `dip::SingularValues`, `dip::SingularValueDecomposition`, `dip::Identity`,
  `dip::SumTensorElements`, `dip::ProductTensorElements`, `dip::AllTensorElements`, `dip::AnyTensorElement`,
  `dip::MaximumTensorElement`, `dip::MaximumAbsTensorElement`, `dip::MinimumTensorElement`,
  `dip::MinimumAbsTensorElement`, `dip::MeanTensorElement`, `dip::GeometricMeanTensorElement`,
  `dip::SortTensorElements`, `dip::SortTensorElementsByMagnitude`,
  `dip::Hypot`.

- New microscopy functions: `dip::BeerLambertMapping`, `dip::InverseBeerLambertMapping`, `dip::UnmixStains`,
  `dip::MixStains`.

- New segmentation functions: `dip::StochasticWatershed`, `dip::WatershedMinima`, `dip::WatershedMaxima`,
  `dip::MinimumVariancePartitioning`, `dip::OtsuThreshold`, `dip::MinimumErrorThreshold`, `dip::TriangleThreshold`,
  `dip::BackgroundThreshold`, `dip::VolumeThreshold`, `dip::MultipleThresholds`.

- New transform functions: `dip::OptimalFourierTransformSize`, `dip::RieszTransform`, `dip::StationaryWaveletTransform`.

- There is also a lot of new functionality in the library infrastructure, which we cannot all list here.
  See \ref infrastructure.

[//]: # (--------------------------------------------------------------)

\section changes_dipimage Changes from DIPimage 2

- The `dip_image` object has changed completely internally. Pixel data is stored differently:
  tensor images have all samples in the same MATLAB array. Complex images are stored as a
  single real matrix, with real and complex samples next to each other (this translates
  much more nicely to *DIPlib*, where complex data is stored in that same way).

  Consequently, the `dip_image_array` pseudo-class no longer exists. Both scalar and tensor
  images report to be of class `dip_image`. If you need to store multiple images in one
  object, create a cell array instead.

  Additionally, the tensor no longer can hold arbitrary number of dimensions, it is limited
  to vectors and matrices, as it is in *DIPlib*. I don't expect this to impact any user's code,
  and I'd be happy to hear if you actually used tensors with more than two dimensions.

  We tried keeping how the `dip_image` object is used as similar as possible to how it was
  in *DIPimage 2*, so that users need not change their code. Nonetheless, some changes
  must occur:

  - The `size` method was discouraged, we asked users to use `imsize` and `imarsize` instead.
    Those two functions work as before, except that they don't return 1 for dimensions that
    don't exist. `size` now always works like `imsize`, except that it always returns at least
    two values, and will return 1 for non-existing dimensions. This makes it similar to the
    default `size`, and plays nice with the `whos` command. `tensorsize` is like `imarsize`,
    but with a more sensical name, since image arrays no longer exist.

  - `end` was discouraged as well, as the meaning changed from `dip_image` to `dip_image_array`.
    Now `end` will always work for spatial dimensions, and cannot be used with tensor indexing.
    That is, `end` will work in `()` indexing, but not in `{}` indexing.

  - Methods that changed behavior for scalar and tensor images (i.e. `sum`, `max`, etc.) now
    always operate only along spatial dimensions. To sum over the tensor dimension, convert
    it to a spatial dimension using `tensortospatial`.

  - Likewise, methods such as `cat` used to concatenate images in an `dip_image_array`, and
    consequently concatenated the various tensor elements as separate images. These functions
    now all work along spatial dimensions only, leaving the tensor dimension unaffected.

  - The `reshape` method now takes pixels column-wise instead of row-wise from the input. This
    is the natural way of doing it, as it doesn't require data copy. I don't remember why it
    was row-wise in *DIPimage 2*, and I presume there are few (if any) programs that
    depend on the old behavior.

  - Related to the previous point, `squeeze` now might reorder dimensions. But it's cheaper
    this way!

  - There are slight changes to how array data types are handled when converting to a `dip_image`
    object and when using an array as input to a *DIPimage* function. Most importantly, the
    *DIPlib* interface translates double-precision scalar arrays to 0D images of a single-precision
    type as long as that does not cause overflow or underflow. This typically yields the expected
    results. To avoid this behavior, explicitly cast to `dip_image`. For example: `img + 1` vs
    `img + dip_image(1)`.

  - The `inner` and `outer` methods no longer exist, use `cross` and `dot`.

  - New methods: `clone`, `cosh`, `cumsum`, `erfc`, `flip`, `gammaln`, `iscomplex`, `issigned`, `isunsigned`,
    `numArgumentsFromSubscript`, `numberchannels`, `numpixels`, `numtensorel`, `sinh`, `slice_ex`, `slice_in`,
    `slice_op`, `spatialtotensor`, `swapdim`, `tanh`, `tensorfun`, `tensorsize`, `tensortospatial`.

  - `eig_largest` has been moved to the toolbox directory.

  - `besselj`, `length` and `unique` are no longer methods of `dip_image`.

- Related to the tensor image changes above, `newimar` and `imarfun` are now in the `alias`
  subdirectory, and are identical to the new functions `newtensorim` and `tensorfun`.

- The `dip_measurement` object has changed completely internally. The interface is identical
  except:

  - It is not (yet?) possible to add a feature.

  - It is no longer possible to convert to/from a `struct`. However, it is possible to convert
    to a `table`.

  - Fixed a bug: `msr.featureID` now returns an array that is transposed w.r.t. previous versions.
    This was a bug that we never fixed because of backwards compatibility, we took this
    opportunity to fix it. Now we have total consistency: no matter how the measurement data are
    extracted or converted, objects are always rows, and features are always columns.

  - For consistency, `msr.ID` now returns object IDs as a column vector (objects are rows).

  - A new method, `remap`, implements what used to be the independent function `msr_remap`.
    `msr_remap` is in the `alias` subdirectory.

- The interactive image display window from `dipshow` has had some internal changes.
  User-visible changes are:

   - Three new options in the "Mappings" menu for 3D and 4D images, labeled "Slice", "Max projection"
     and "Mean projection". These allow to display max or mean projection instead of the default
     thin slice.

   - Removal of the "Orientation testing" (2D), and "Max projection" and "Sum projection" (3D)
     options in the "Actions" menu. The two functions that implemented this functionality,
     `diporien` and `dipprojection`, have also been removed.

   - A new function `viewslice` can be used to display any image (including tensor images and
     higher-dimensional images) in \ref viewer. This is an alternative way to examine images,
     but none of the tools to programmatically interact with images displayed through `dipshow`
     will work with this viewer.

- Configuration settings accessed through `dipsetpref` and `dipgetpref` have been changed since
  *DIPimage 2*:

   - Added settings:

       - '`DisplayFunction`' can be set to either '`dipshow`' (the default), or '`viewslice`',
         and determines which of these functions is invoked by default when the `dip_image/display`
         method is invoked (see the
         [*DIPimage* User Manual](https://diplib.github.io/diplib-docs/DIPimageUserManual.pdf)
         for more detauls).

   - Changed settings:

       - '`KeepDataType`', which changes how the output data type for arithmetic operations is
         chosen, has the same intent but does not always make the same choice. For example
         the result is different when mixing signed and unsigned integers. (see the
         [*DIPimage* User Manual](https://diplib.github.io/diplib-docs/DIPimageUserManual.pdf)
         for details on the current logic).

   - Removed settings:

       - '`BoundaryCondition`', '`Truncation`', and '`DerivativeFlavour`', because relevant
         functions take these parameters as optional input arguments.

       - '`MorphologicalFlavour`', as it didn't seem useful (the associated function
         `dip_morph_flavour` has also been removed).

       - '`ComputationLimit`', because image arithmetic is now always performed by *DIPlib*.

       - '`ConflictingPixelSize`' and '`InconsistentPixelSize`', because the *DIPlib* library
         keeps track of pixel sizes and handles these situations in a fixed manner.

       - '`CommandFilePath`', because the *DIPimage* GUI no longer probes functions to
         put them in the menus.

       - '`DebugMode`', as *DIPlib* stack traces are shown or not depending on a
         compile-time flag.

       - '`FFTtype`', because *DIPlib* can optionally be compiled to use *FFTW* for the FFT.

       - '`FastSubscriptedAssignment`', introduced recently in *DIPimage 2.9*, because the
         related functionality has not been ported over (yet?).

- Many filters now have a boundary condition parameter. In *DIPimage 2*, one would change
  the boundary condition through a global parameter, which no longer exists. If you never
  touched the global parameter, nothing should change for you. If you did change this global
  parameter in a program, you now need to pass the value to the relevant functions instead.

- Many functions have been added to match new functionality in *DIPlib*, as well as previous
  functionality that was not accessible from MATLAB. Some old functions have gained additional
  parameters to expose more functionality; we tried to do so without breaking backwards
  compatibility.

- Some functions have changed in a way that is not backwards compatible, either by having a
  different interface or different behavior:

  - `dilation_se`, `erosion_se`, etc. are now folded into `dilation`, `erosion`, etc. For
    functions with an `_se` appended to the name, remove the `_se`. Alternatively, the `alias`
    sub-directory contains these names and forwards the calls to the correct functions.

  - `rankmax_opening` and `rankmin_closing` now have `rank` as the second parameter, instead
    of `percentile`. This should make them match the literature better, and make them more
    usable.

  - `smooth` is no longer relevant, moved to the `alias` directory.

  - `derivative` has the 2<sup>nd</sup> and 3<sup>rd</sup> arguments switched, it makes more
    sense having the order first.

  - `mdhistogram` has fewer options, but should still be able to produce the same results as
    previously. In `hist2image`, the `coords` input is transposed w.r.t. previous versions,
    for consistency: each row is interpreted as a vertex.

  - `resample` and `shift` shift the image in the opposite direction from what it did in
    *DIPimage 2*, where the shift was unintuitive.

  - `affine_trans` rotates in the opposite direction it did before, to match the interpretation
    of angles in the rest of the toolbox. The function now also accepts an affine transformation
    matrix to direct the transformation.

  - `readim` and `writeim` work differently now, in part because *DIPlib* natively only supports
    two file types now. The `file_info` struct output for `readim` has changed somewhat. The last
    two input parameters to the old `writeim` are no longer supported (`compression` and `physDim`):
    to change the compression method, call `writeics` or `writetiff` directly; the pixel size is
    always given by the image, use `dip_image/pixelsize` to set it. `readics` and `readtiff` are
    the interfaces to the corresponding *DIPlib* file reading functions.

  - `countneighbours` has been renamed to `countneighbors` for consistency in spelling.

  - `mse`, `mae` and several similar functions have been collected in a new function `errormeasure`.
    The old functions are still available in the `alias` sub-directory.

  - `slice_in` and `slice_ex` specify dimensions starting at 1 (not at 0 as previously). This
    change is for consistency with the rest of the toolbox. These functions now reorder dimensions
    in a way consistent with `dip_image/squeeze` (removing dimension 1 causes dimension 3 to become
    dimension 1). `slice_in`, `slice_ex` and `slice_op` have been made into `dip_image` methods,
    for efficiency.

  - `get_subpixel` no longer supports the `spline` interpolation method, this string is still
    accepted, but uses the `cubic` method.

  - `structuretensor3d` has been moved to the `alias` directory, `structuretensor` now works for any
    number of dimensions (with special support for 2D and 3D images).

  - `curvature_thirion` and `isophote_curvature` have been moved to the `alias` directory. The function
    `curvature` now takes '`thirion`' and '`isophote`' as options. `orientation4d` has been moved to the
    `alias` directory. A new function `orientation` generalizes it to arbitrary dimensionality.

  - `granulometry` has changed, but it is still possible to call it the old way. However, the parameters
    in this old syntax are interpreted to match the new capabilities of this function. `'usegrey'` and
    `'verbose'` options no longer have an effect. Default values have changed a bit.

  - `ht` no longer exists. Use `riesz` instead. The Riesz transform is the multi-dimensional generalization
    of the 1D Hilbert transform. See also the new function `monogenicsignal`.

  - `nconv` no longer exists, the new function `normconv` will compute the normalized convolution with
    a Gaussian, as well as estimate first derivatives using a Gaussian normalized convolution.

  - `dt` has a new algorithm, which is used by default. It gives exact results and works for any number of
    dimensions, and it should be faster than the previous default algorithm. The old default algorithm can
    be executed with `dt(...,'fast')`.

  - `gdt` has a new algorithm, which is used by default. It approximates Euclidean distances better. The old
    default algorithm can be executed with `gdt(...,3)`. The second output image is no longer produced. A
    new, optional argument specifies a mask image to further constrain paths.

  - `testobject` has a changed interface, the input argument order has changed and most arguments are now
    name-value pairs. Its functionality has been greatly extended.

  - `radoncircle` has a different implementation with different capabilities and a different interface.

- New functions not mentioned above: `abssqr`, `areaopening`, `asf`, `cell2im`, `cluster`,
  `cornerdetector`, `coordinates`, `distancedistribution`, `drawshape`, `extendregion`,
  `getmaximumandminimum`, `getsamplestatistics`, `im2cell`, `lee`, `linedetector`, `loggabor`,
  `pathopening`, `perobjecthist`, `psf`, `quantize`, `select`, `semivariogram`, `setborder`,
  `skew`, `smallobjectsremove`, `thetatheta`, `traceobjects`.
  Use `help <functionname>` in MATLAB to learn what these functions provide.

- `jacobi` moved to the `alias` directory, `eig` does it better now.

- Old functions that used to be in the `alias` directory are no longer. We recommend that
  you correct affected code, but if you want, you can always create those aliases again.

- If you customized the menus in the *DIPimage* GUI, you will have to update your `localdipmenus.m`
  file. If you wrote your own functions that integrated in the GUI, you'll have to do so through
  your `localdipmenus.m` now. Preference setting '`CommandFilePath`' no longer exists,
  `getparams` no longer exists, and functions are no longer probed with '`DIP_GetParamList`'.
