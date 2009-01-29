CMGUI Computed fields
=====================

.. |xi|     unicode:: U+003BE .. GREEK SMALL LETTER XI
.. |sub1|  unicode:: U+02081 .. SUBSCRIPT ONE
.. |sub2|  unicode:: U+02082 .. SUBSCRIPT TWO
.. |sub3|  unicode:: U+02083 .. SUBSCRIPT THREE
.. _fields: http://www.cmiss.org/cmgui/wiki/CMGUIFields
.. _graphical settings: http://www.cmiss.org/cmgui/wiki/UsingCMGUIGraphicalSettings
.. _glyphs: http://www.cmiss.org/cmgui/wiki/VisualizingFieldsAtPointsUsingGlyphs
.. _surfaces: http://www.cmiss.org/cmgui/wiki/VisualizingElementFieldsUsingSurfaces
.. _manifolds: http://en.wikipedia.org/wiki/Manifold
.. _graphics window: http://www.cmiss.org/cmgui/wiki/UsingCMGUITheGraphicsWindow
.. _scene editor: http://www.cmiss.org/cmgui/wiki/UsingCMGUITheSceneEditorWindow
.. _glyphs: http://www.cmiss.org/cmgui/wiki/VisualizingFieldsAtPointsUsingGlyphs
.. _material editor window: http://www.cmiss.org/cmgui/wiki/UsingCMGUIMaterialEditor
.. _spectrum editor window: http://www.cmiss.org/cmgui/wiki/UsingCMGUISpectrumEditor
.. _example a7: http://cmiss.bioeng.auckland.ac.nz/development/examples/a/a7/index.html

Computed fields in CMGUI
------------------------

CMGUI is able to create fields_ using other fields_ as input; these are known as *computed fields*. 

2d_strain
<deformed_coordinate FIELD_NAME[.COMPONENT_NAME]|none[none]>
<undeformed_coordinate FIELD_NAME[.COMPONENT_NAME]|none[none]>
<fibre_angle FIELD_NAME[.COMPONENT_NAME]|none[none]>
acos
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  add
<fields FIELD_NAME[.COMPONENT_NAME]|none[none] FIELD_NAME[.COMPONENT_NAME]|none[none]>
<scale_factors # #[1 1]>
  and
<fields FIELD_NAME[.COMPONENT_NAME]|none[none] FIELD_NAME[.COMPONENT_NAME]|none[none]>
  asin
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  atan
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  atan2
<fields FIELD_NAME[.COMPONENT_NAME]|none[none] FIELD_NAME[.COMPONENT_NAME]|none[none]>
  basis_derivative
  * The basis_derivative calculates a monomial derivative 
  *  on element based fields.  It is not defined for nodes.  
  *  It allows you to calculate an arbitrary derivative 
  *  by specifying an <order> and a list of <xi_indices> 
  *  of length order.  This derivative then becomes the 
  *  "value" for the field.
<fe_field FIELD_NAME|none[none]>
<order #[1]{>0,integer}>
<xi_indices #[1]>
  binary_dilate_filter
  * The binary_dilate_filter field uses the itk::BinaryDilateImageFilter 
  *  code to produce an output field which is a dilation 
  *  of a binary image (an image where each pixel has 1 
  *  of 2 values).  The region identified by the pixels 
  *  with intensity <dilate_value> is dilate (enlarged), 
  *  by replacing each pixel with a ball.  The size of the 
  *  ball is set by specifying the <radius>. The <field> 
  *  it operates on is usually a thresholded or segmented 
  *  field. See a/testing/image_processing_2D for an example 
  *  of using this field.  For more information see the 
  *  itk software guide.
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<radius #[1]{>=0,integer}>
<dilate_value #[1]>
  binary_erode_filter
  * The binary_erode_filter field uses the itk::BinaryErodeImageFilter 
  *  code to produce an output field which is a erosion 
  *  of a binary image (an image where each pixel has 1 
  *  of 2 values).  The region identified by the pixels 
  *  with intensity <erode_value> is eroded (decreased).  
  *  The erosion effect can be increase by inreasing the 
  *  <radius> value. The <field> it operates on is usually 
  *  a thresholded or segmented field. See a/testing/image_processing_2D 
  *  for an example of using this field.  For more information 
  *  see the itk software guide.
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<radius #[1]{>=0,integer}>
<erode_value #[1]>
  binary_threshold_filter
  * The binary_threshold_filter field uses the itk::BinaryThresholdImageFilter 
  *  code to produce an output field where each pixel has 
  *  one of two values (either 0 or 1). It is useful for 
  *  separating out regions of interest. The <field> it 
  *  operates on is usually a sample_texture field, based 
  *  on a texture that has been created from image file(s).  
  *  Pixels with an intensity range between <lower_threshold> 
  *  and the <upper_threshold> are set to 1, the rest are 
  *  set to 0. See a/testing/image_processing_2D for an 
  *  example of using this field.  For more information 
  *  see the itk software guide.
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<lower_threshold #[0]>
<upper_threshold #[1]>
  canny_edge_detection_filter
  * The canny_edge_detection field uses the itk::CannyEdgeDetectionImageFilter 
  *  code to detect edges in a field. The <field> it operates 
  *  on is usually a sample_texture field, based on a texture 
  *  that has been created from image file(s).  Increasing 
  *  the <variance> smooths the input image more, which 
  *  reduces sensitivity to image noise at the expense of 
  *  losing some detail. Decreasing the <maximum_error> 
  *  also reduces edges detected as the result of noise.  
  *  The <upper_threshold> sets the level which a point 
  *  must be above to use it as the start of the edge. The 
  *  edge will then grow from that point until the level 
  *  drops below the <lower_threshold>. Increasing the <upper_threshold> 
  *  will decrease the number of edges detected. Increasing 
  *  the <lower_threshold> will reduce the length of edges.  
  *  See a/testing/image_processing_2D for an example of 
  *  using this field.
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<variance #[0]>
<maximum_error #[0.01]>
<upper_threshold #[0]>
<lower_threshold #[0]>
  clamp_maximum
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<maximums VALUES>
  clamp_minimum
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<minimums VALUES>
  cmiss_number
  component
< #|FIELD[.COMPONENT_NAME] #|FIELD[.COMPONENT_NAME] #|FIELD[.COMPONENT_NAME] ... >
  compose
  * The value of a compose field is found by evaluating 
  *  the <texture_coordinates_field>, then searching for 
  *  matching values of the <find_element_xi_field> in the 
  *  elements of the <group> and then finally evaluating 
  *  the <calculate_values_field> at this found location.  
  *  By restricting the <element_dimension> you can speed 
  *  up the search and you can specify the outcome if the 
  *  matching values cannot be found in the element <group> 
  *  with <use_point_five_when_out_of_bounds> or <fail_when_out_of_bounds>.  
  *  See a/resample_texture or a/create_slices where the 
  *  compose field is used to find the equivalent coordinate 
  *  in another element to evaluate a texture.
<calculate_values_field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<element_dimension #[0]{>=0,integer}>
<find_element_xi_field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<find_nearest|find_exact>
<group PATH_TO_REGION[/]>
<texture_coordinates_field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<use_point_five_when_out_of_bounds|fail_when_out_of_bounds>
  composite
< #|FIELD[.COMPONENT_NAME] #|FIELD[.COMPONENT_NAME] #|FIELD[.COMPONENT_NAME] ... >
  connected_threshold_filter
  * The connected_threshold_filter field uses the itk::ConnectedThresholdImageFilter 
  *  code to segment a field. The <field> it operates on 
  *  is usually a sample_texture field, based on a texture 
  *  that has been created from image file(s).  The segmentation 
  *  is based on a region growing algorithm which requires 
  *  at least one seed point.  To specify the seed points 
  *  first set the <num_seed_points> and the <dimension> 
  *  of the image.  The <seed_points> are a list of the 
  *  coordinates for the first and any subsequent seed points.  
  *  Starting from the seed points any neighbouring pixels 
  *  with an intensity between <lower_threshold> and the 
  *  <upper_threshold> are added to the region.  Pixels 
  *  within the region have their pixel intensity set to 
  *  <replace_value> while the remaining pixels are set 
  *  to 0. See a/testing/image_processing_2D for an example 
  *  of using this field.  For more information see the 
  *  itk software guide.
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<lower_threshold #[0]>
<upper_threshold #[1]>
<replace_value #[1]>
<num_seed_points #[0]{>0,integer}>
<dimension #[2]{>0,integer}>
<seed_points VALUES>
  constant
  * A constant field may be defined as having one or more 
  *  components.  Each of the <values> listed is used to 
  *  asign a constant value to the corresponding field component. 
  *  Fields with more than 1 component can be used to represent 
  *  vectors or matrices.  An m by n matrix requires (m*n) 
  *  components and the components of the matrix are listed 
  *  row by row.
< VALUES>
  coordinate_transformation
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  cos
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  cross_product
<dimension #[3]{>0,integer}>
<fields FIELD_NAME[.COMPONENT_NAME]|none[none] FIELD_NAME[.COMPONENT_NAME]|none[none]>
  cubic_texture_coordinates
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  curl
<coordinate FIELD_NAME[.COMPONENT_NAME]|none[none]>
<vector FIELD_NAME[.COMPONENT_NAME]|none[none]>
  curvature_anisotropic_diffusion_filter
  * The curvature_anisotropic_filter field uses the itk::CurvatureAnisotropicImageFilter 
  *  code to smooth a field to reduce noise (or unwanted 
  *  detail) while preserving edges. The <field> it operates 
  *  on is usually a sample_texture field, based on a texture 
  *  that has been created from image file(s).  The smoothing 
  *  is accomplished by numerically solving a modified curvature 
  *  diffusion equation.  The accuracy of the numerical 
  *  solution can be adjusted by varying the <time_step> 
  *  and <num_iterations> used.  The <conductance> is a 
  *  parameter used by the diffusion equation.  A high value 
  *  of conductance causes the image to diffuse (smooth) 
  *  more, while a low value puts more emphasis on preserving 
  *  features.  Typical conductance values are often in 
  *  the range 0.5 to 2. See a/testing/image_processing_2D 
  *  for an example of using this field. For more information 
  *  see the itk software guide.
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<time_step #[0.125]>
<conductance #[3]>
<num_iterations #[5]{>=0,integer}>
  curve_lookup
<curve CURVE_NAME|none[none]>
<source FIELD_NAME[.COMPONENT_NAME]|none[none]>
  derivative
  * The derivative field has two modes of operation.  For 
  *  normal finite element fields it simply promotes the 
  *  derivative values corresponding to <xi_index> calculated 
  *  by the input <field> to be the field values.  These 
  *  derivatives are with respect to xi. If the input <field> 
  *  cannot cannot calculate element based derivatives then 
  *  if the input field has a native resolution then this 
  *  field uses the ITK DerivativeImageFilter to calculate 
  *  a pixel based derivative at that same resolution.  
  *  The derivative filter will use the image pixel physical 
  *  spacing if that is defined for ITK.  Note that as the 
  *  derivative is a signed value you may want to offset 
  *  and scale the resultant values if you intend to store 
  *  them in an unsigned pixel format.
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<xi_index #[1]{>0,integer}>
  derivative_filter
  * The derivative_filter field uses the itk::DerivativeImageFilter 
  *  code to calculate the derivative of a field in a particular 
  *  direction. The <field> it operates on is usually a 
  *  sample_texture field, based on a texture that has been 
  *  created from image file(s).  The <order> paramater 
  *  sets the order of the derivative and the <direction> 
  *  parameter is an integer value that specifies the direction 
  *  to evaluate the derivative in.  0 corresponds to the 
  *  x direction. See a/testing/image_processing_2D for 
  *  an example of using this field. For more information 
  *  see the itk software guide.
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<order #[1]{>=0,integer}>
<direction #[1]{>=0,integer}>
  discrete_gaussian_filter
  * The discrete_gaussian_filter field uses the itk::DiscreteGaussianImageFilter 
  *  code to smooth a field. It is useful for removing noise 
  *  or unwanted detail.  The <field> it operates on is 
  *  usually a sample_texture field, based on a texture 
  *  that has been created from image file(s).  The effect 
  *  of applying a discrete gaussian image filter is that 
  *  a pixel value is based on a weighted average of surrounding 
  *  pixel values, where the closer the pixel the more weight 
  *  its value is given. Increasing the <variance> increases 
  *  the width of the gaussian distribution used and hence 
  *  the number of pixels used to calculate the weighted 
  *  average. This smooths the image more.  A limit is set 
  *  on the <max_kernel_width> used to approximate the guassian 
  *  to ensure the calculation completes.  See a/testing/image_processing_2D 
  *  for an example of using this field. For more information 
  *  see the itk software guide.
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<variance #[1]>
<max_kernel_width #[4]{>0,integer}>
  divergence
<coordinate FIELD_NAME[.COMPONENT_NAME]|none[none]>
<vector FIELD_NAME[.COMPONENT_NAME]|none[none]>
  divide_components
<fields FIELD_NAME[.COMPONENT_NAME]|none[none] FIELD_NAME[.COMPONENT_NAME]|none[none]>
  dot_product
<fields FIELD_NAME[.COMPONENT_NAME]|none[none] FIELD_NAME[.COMPONENT_NAME]|none[none]>
  edit_mask
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<edit_mask VALUES>
  eigenvalues
  * An eigenvalues field returns the n eigenvalues of an 
  *  (n * n) square matrix field.  Here, a 9 component source 
  *  field is interpreted as a (3 * 3) matrix with the first 
  *  3 components being the first row, the next 3 components 
  *  being the middle row, and so on.  The related eigenvectors 
  *  field can extract the corresponding eigenvectors for 
  *  the eigenvalues. See a/large_strain for an example 
  *  of using the eigenvalues and eigenvectors fields.
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  eigenvectors
  * An eigenvectors field returns vectors corresponding 
  *  to each eigenvalue from a source eigenvalues field.  
  *  For example, if 3 eigenvectors have been computed for 
  *  a (3 * 3) matrix = 9 component field, the eigenvectors 
  *  will be a 9 component field with the eigenvector corresponding 
  *  to the first eigenvalue in the first 3 components, 
  *  the second eigenvector in the next 3 components, and 
  *  so on.  See a/large_strain for an example of using 
  *  the eigenvalues and eigenvectors fields.
<eigenvalues FIELD_NAME[.COMPONENT_NAME]|none[none]>
  embedded
<element_xi FIELD_NAME|none[none]>
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  equal_to
<fields FIELD_NAME[.COMPONENT_NAME]|none[none] FIELD_NAME[.COMPONENT_NAME]|none[none]>
  exp
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  fast_marching_filter
  * The fast_marching_filter field uses the itk::FastMarchingImageFilter 
  *  code to segment a field. The segmentation is based 
  *  on a level set algorithm.  The <field> it operates 
  *  on is used as a speed term, to govern where the level 
  *  set curve grows quickly.  The speed term is usually 
  *  some function (eg a sigmoid) of an image gradient field.  
  *  The output field is a time crossing map, where the 
  *  value at is each pixel is the time take to reach that 
  *  location from the specified seed points.  Values typically 
  *  range from 0 through to extremely large (10 to the 
  *  38).  To convert the time cross map into a segmented 
  *  region use a binary threshold filter. To specify the 
  *  seed points first set the <num_seed_points> and the 
  *  <dimension> of the image.  The <seed_points> are a 
  *  list of the coordinates for the first and any subsequent 
  *  seed points.   It is also possible to specify non-zero 
  *  initial <seed_values> if desired and to set the <output_size> 
  *  of the time crossing map. See a/segmentation for an 
  *  example of using this field.  For more information 
  *  see the itk software guide.
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<stopping_value #[100]>
<num_seed_points #[1]{>0,integer}>
<dimension #[2]{>0,integer}>
<seed_points # #[0.5 0.5]>
<seed_values #[0]>
<output_size # #[128 128]>
  fibre_axes
<coordinate FIELD_NAME[.COMPONENT_NAME]|none[none]>
<fibre FIELD_NAME[.COMPONENT_NAME]|none[none]>
  finite_element
<anatomical|coordinate[coordinate]|field>
<component_names NAME NAME NAME["" "" ""]>
<double|element_xi|real[real]|float|integer|short|string|unsigned|url>
<number_of_components #[3]{>0,integer}>
  function
  * The value of a function field is found by evaluating 
  *  the <source_field> values, and then evaluating the 
  *  <result_field> with respect to the <reference_field> 
  *  using the values from the source field.  The sequence 
  *  of operations <reference_field> to <result_field> become 
  *  a function operating on the input <source_field> values.  
  *  Either the number of components in the <source_field> 
  *  and <reference_field> should be the same, and then 
  *  the number of components of this <field> will be the 
  *  same as the number of components in the <result_field>, 
  *  or if the <reference_field> and <result_field> are 
  *  scalar then the function operation will be applied 
  *  as many times as required for each component in the 
  *  <source_field> and then this <field> will have as many 
  *  components as the <source_field>.
<reference_field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<result_field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<source_field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  gradient
<coordinate FIELD_NAME[.COMPONENT_NAME]|none[none]>
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  gradient_magnitude_recursive_gaussian_filter
  * The gradient_magnitude_recursive_filter field uses 
  *  the itk::GradientMagnitudeRecursiveImageFilter code 
  *  to compute the magnitude of the image gradient at each 
  *  location in the field. It is useful for identifying 
  *  regions where the pixel intensities change rapidly.  
  *  The <field> it operates on is usually a sample_texture 
  *  field, based on a texture that has been created from 
  *  image file(s).  The filter first smooths the image 
  *  using a discrete gaussian image subfilter before calculating 
  *  the gradient and magnitudes.  Increasing <sigma> increases 
  *  the width of the gaussian distribution used during 
  *  the smoothing and hence the number of pixels used to 
  *  calculate the weighted average. This smooths the image 
  *  more.  See a/testing/image_processing_2D for an example 
  *  of using this field.  For more information see the 
  *  itk software guide.
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<sigma #[2]>
  greater_than
<fields FIELD_NAME[.COMPONENT_NAME]|none[none] FIELD_NAME[.COMPONENT_NAME]|none[none]>
  histogram_filter
  * The histogram_filter field uses the itk::ImageToHistogramGenerator 
  *  code to generate binned values representing the relative 
  *  frequency of the various pixel intensities.  There 
  *  should be a number_of_bins for each component direction, 
  *  and so the total number of bins will be a product of 
  *  these, so that for a 3 component image you would get 
  *  a volume histogram.  If you wanted a histogram for 
  *  a single component then set the number_of_bins for 
  *  the other components to 1.
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<number_of_bins #[64]{>0,integer}>
<marginal_scale #[10]>
  if
  * The if field uses three input fields.  The first field 
  *  is evaluated and for each component if the value of 
  *  the component is not zero (== true) then the value 
  *  for that component is copied from the second field.  
  *  Otherwise (the first field value was zero == false) 
  *  the value for that component is copied from the third field
<fields FIELD_NAME[.COMPONENT_NAME]|none[none] FIELD_NAME[.COMPONENT_NAME]|none[none] FIELD_NAME[.COMPONENT_NAME]|none[none]>
  image
  * The image field allows you to look up the values of 
  *  a <texture>.  This sample_texture interface wraps an 
  *  existing texture in a image field.  The resulting field 
  *  will have the same number of components as the texture 
  *  it was created from.  The <coordinates> field is used 
  *  as the texel location, with values from 0..texture_width, 
  *  0..texture_height and 0..texture_depth valid coordinates 
  *  within the image.  Normally the resulting colour values 
  *  are real values for 0 to 1.  The <minimum> and <maximum> 
  *  values can be used to rerange the colour values.  The 
  *  <native_texture> or <not_native_texture> flag indicates 
  *  whether this sample texture computed field will supply 
  *  this textures dimensions as the default resolution 
  *  to a modify texture evalutate_image command that is 
  *  using this field.  This is normally what you want but 
  *  the flag gives you the ability to discriminate which 
  *  texture should be used in a pipeline of fields.  See 
  *  examples a/reimage, a/create_slices and a/image_sampling.  
<coordinates FIELD_NAME[.COMPONENT_NAME]|none[none]>
<maximum #[1]>
<minimum #[0]>
<native_texture>
<not_native_texture>
<texture TEXTURE_NAME|none[none]>
  image_resample
  * The image_resample field resamples the field to a new 
  *  user specified size. It is especially useful for resizing 
  *  image based fields.  The new size of the field is specified 
  *  by using the <sizes> option with a list of values for 
  *  the new size in each dimension.  See a/testing/image_processing_2D 
  *  for an example of using this field.
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<sizes VALUES>
  integration
<coordinate FIELD_NAME[.COMPONENT_NAME]|none[none]>
<integrand FIELD_NAME[.COMPONENT_NAME]|none[constant_1.0]>
<magnitude_coordinates>
<region PATH_TO_REGION[/]>
<seed_element ELEMENT_NUMBER>
<update_time_integration #[0]>
  is_defined
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  less_than
<fields FIELD_NAME[.COMPONENT_NAME]|none[none] FIELD_NAME[.COMPONENT_NAME]|none[none]>
  log
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  magnitude
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  matrix_invert
  * A matrix_invert field returns the inverse of a square 
  *  matrix.  Here, a 9 component source field is interpreted 
  *  as a (3 * 3) matrix with the first 3 components being 
  *  the first row, the next 3 components being the middle 
  *  row, and so on.  See a/current_density for an example 
  *  of using the matrix_invert field.
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  matrix_multiply
  * A matrix_mutliply field calculates the product of two 
  *  matrices, giving a new m by n matrix.  The product 
  *  is represented as a field with a list of (m * n) components.   
  *  The components of the matrix are listed row by row.  
  *  The <number_of_rows> m is used to infer the dimensions 
  *  of the source matrices.  The two source <fields> are 
  *  multiplied, with the components of the first interpreted 
  *  as a matrix with dimensions m by s and the second as 
  *  a matrix with dimensions s by n.  If the matrix dimensions 
  *  are not consistent then an error is returned.  See 
  *  a/curvature for an example of using the matrix_multiply 
  *  field.
<number_of_rows #[1]{>0,integer}>
<fields FIELD_NAME[.COMPONENT_NAME]|none[none] FIELD_NAME[.COMPONENT_NAME]|none[none]>
  matrix_to_quaternion
  * A computed field to convert a 4x4 matrix to a quaternion.  
  *  components of the matrix should be read in as follow           
  *  0   1   2   3                                              
  *  4   5   6   7                                              
  *  8   9   10  11                                             
  *  12  13  14  15                                         

<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  mean_filter
  * The mean_filter field uses the itk::MeanImageFilter 
  *  code to replace each pixel with the mean intensity 
  *  of the pixel and its surrounding neighbours.  It is 
  *  useful for reducing the level of noise.   The <field> 
  *  it operates on is usually a sample_texture field, based 
  *  on a texture that has been created from image file(s).   
  *  The size of the neighbourhood of pixels used to calculate 
  *  the mean is determined be a list of <radius_sizes>, 
  *  one value for each dimension.  Each radius size sets 
  *  how many pixels to include either side of the central 
  *  pixel for the corresponding dimension. If radius values 
  *  are increased, more neighbouring pixels are included 
  *  and the image becomes smoother. See a/testing/image_processing_2D 
  *  for an example of using this field. For more information 
  *  see the itk software guide.
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<radius_sizes VALUES>
  multiply_components
<fields FIELD_NAME[.COMPONENT_NAME]|none[none] FIELD_NAME[.COMPONENT_NAME]|none[none]>
  nodal_lookup
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<node #[NOT SET]>
<region PATH_TO_REGION[/]>
  node_value
<fe_field FIELD_NAME|none[none]>
<value[value]|d/ds1|d/ds2|d/ds3|d2/ds1ds2|d2/ds1ds3|d2/ds2ds3|d3/ds1ds2ds3>
<version #[1]{>0,integer}>
  normalise
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  offset
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<offsets VALUES>
  or
<fields FIELD_NAME[.COMPONENT_NAME]|none[none] FIELD_NAME[.COMPONENT_NAME]|none[none]>
  power
<fields FIELD_NAME[.COMPONENT_NAME]|none[none] FIELD_NAME[.COMPONENT_NAME]|none[none]>
  projection
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<number_of_components #[0]{>0,integer}>
<projection_matrix #[0]>
  quaternion_SLERP
  * A 4 components quaternion field. The components of 
  *  the quaternion field are expected to be the w, x, y, 
  *  z componentsof a quaternion (4 components in total). 
  *  The quaternion field  isevaluated and interpolated 
  *  using SLERP at a normalised time between twoquaternions 
  *  (read in from the exnode generally). This quaternion 
  *  fieldcan be convert to a matrix with quaternion_to_matrix 
  *  field, the resultingmatrix can be used to create a 
  *  smooth time dependent rotation for an objectusing the 
  *  quaternion_to_matrix field. This field must be define 
  *  directly fromexnode file or from a matrix_to_quaternion 
  *  field
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<node #[NOT SET]>
<region PATH_TO_REGION[/]>
  quaternion_to_matrix
  * A computed field to convert a quaternion (w,x,y,z) 
  *  to a 4x4 matrix,
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  region_mean
<region PATH_TO_REGION>
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  region_rms
<region PATH_TO_REGION>
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  region_sum
<region PATH_TO_REGION>
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  rescale_intensity_filter
  * The rescale_intensity_filter field uses the itk::RescaleIntensityImageFilter 
  *  code to linearly scale the pixel intensity to vary 
  *  between the specified minimum and maximum intensity 
  *  values.  The <field> it operates on is usually a sample_texture 
  *  field, based on a texture that has been created from 
  *  image file(s). Set the <output_min> and <output_max> 
  *  values to define the new range to scale to.  See a/testing/image_processing_2D 
  *  for an example of using this field. For more information 
  *  see the itk software guide.
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<output_min #[0]>
<output_max #[255]>
  sample_texture
  * The image field allows you to look up the values of 
  *  a <texture>.  This sample_texture interface wraps an 
  *  existing texture in a image field.  The resulting field 
  *  will have the same number of components as the texture 
  *  it was created from.  The <coordinates> field is used 
  *  as the texel location, with values from 0..texture_width, 
  *  0..texture_height and 0..texture_depth valid coordinates 
  *  within the image.  Normally the resulting colour values 
  *  are real values for 0 to 1.  The <minimum> and <maximum> 
  *  values can be used to rerange the colour values.  The 
  *  <native_texture> or <not_native_texture> flag indicates 
  *  whether this sample texture computed field will supply 
  *  this textures dimensions as the default resolution 
  *  to a modify texture evalutate_image command that is 
  *  using this field.  This is normally what you want but 
  *  the flag gives you the ability to discriminate which 
  *  texture should be used in a pipeline of fields.  See 
  *  examples a/reimage, a/create_slices and a/image_sampling.  
<coordinates FIELD_NAME[.COMPONENT_NAME]|none[none]>
<maximum #[1]>
<minimum #[0]>
<native_texture>
<not_native_texture>
<texture TEXTURE_NAME|none[none]>
  scale
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<scale_factors VALUES>
  sigmoid_filter
  * The sigmoid_filter field uses the itk::SigmoidImageFilter 
  *  code to nonlinearly scale the pixel intensity to vary 
  *  between the specified minimum and maximum intensity 
  *  values according to a sigmoid curve. It is useful for 
  *  focusing attention on a particular set of values while 
  *  providing a smooth transition at the borders of the 
  *  range. The <field> it operates on is usually a sample_texture 
  *  field, based on a texture that has been created from 
  *  image file(s).  Intensity values are rescaled to vary 
  *  from the <minimum> to the <maximum> value using a sigmoid 
  *  curve which has a width and centre defined by <alpha> 
  *  and <beta>.  Increasing the <alpha> parameter widens 
  *  the curve while increasing the <beta> parameter moves 
  *  the centre of the curve to the right. See a/testing/image_processing_2D 
  *  for an example of using this field.  For more information 
  *  see the itk software guide.
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<minimum #[0]>
<maximum #[1]>
<alpha #[0.25]>
<beta #[0.5]>
  sin
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  sqrt
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  string_constant <STRINGS>
  sum_components
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<weights VALUES>
  tan
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  threshold_filter
  * The threshold_filter field uses the itk::ThresholdImageFilter 
  *  code to change or identify pixels based on whether 
  *  they are above or below a particular intensity value. 
  *  The <field> it operates on is usually a sample_texture 
  *  field, based on a texture that has been created from 
  *  image file(s).  To specify an intensity range to change 
  *  use one of the three threshold modes: <below>, <above> 
  *  or <outside>.  Pixels within the specified range are 
  *  changed to the <outside_value> intensity, the other 
  *  pixels are left unchanged.  For the <below> mode all 
  *  pixels are changed that are below the <below_value>.  
  *  For the <above> mode all pixels are changed that are 
  *  above the <above_value>.  For the <outside> mode all 
  *  pixels are changed that are oustide the range <below_value> 
  *  to <above_value> .  See a/testing/image_processing_2D 
  *  for examples of using this field. For more information 
  *  see the itk software guide.
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<below[below]|above|outside>
<outside_value #[0]>
<below_value #[0.5]>
<above_value #[0.5]>
  time_lookup
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<time_field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  time_value
  transpose
  * A transpose field returns the transpose of a source 
  *  matrix field.  If the source <field> has (m * n) components 
  *  where m is specified by <source_number_of_rows> (with 
  *  the first n components being the first row), the result 
  *  is its (n * m) transpose.  See a/current_density for 
  *  an example of using the matrix_invert field.
<source_number_of_rows #[1]{>0,integer}>
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
  vector_coordinate_transformation
<coordinate FIELD_NAME[.COMPONENT_NAME]|none[none]>
<vector FIELD_NAME[.COMPONENT_NAME]|none[none]>
  window_projection
<field FIELD_NAME[.COMPONENT_NAME]|none[none]>
<ndc_projection|texture_projection[texture_projection]|viewport_projection|inverse_ndc_projection|inverse_texture_projection|inverse_viewport_projection>
<pane_number #[1]{>0,integer}>
<window WINDOW_NUMBER>
  xi_coordinates
  xi_texture_coordinates
<region PATH_TO_REGION[/]>
<seed_element ELEMENT_NUMBER>
  xor
<fields FIELD_NAME[.COMPONENT_NAME]|none[none] FIELD_NAME[.COMPONENT_NAME]|none[none]>
