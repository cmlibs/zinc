/**
 * C++ interfaces for graphics_object.cpp
 */

#ifndef GRAPHICS_OBJECT_HPP
#define GRAPHICS_OBJECT_HPP

class Render_graphics;

/*****************************************************************************//**
 * Specifies the type of storage to be used for the vertex buffer array.
 * As vertices are added to the array they will be organised in memory
 * according to this type, so that they are efficiently formatted when the
 * vertex buffer memory pointers are retrieved.
*/
enum Graphics_vertex_array_type
{
	/** Each type of vertex attribute is added to a buffer for vertices of just that type.
	 * All attributes are stored as float values, except element indices which are
	 * stored by recording the sizes of each array and the first index suitable for
	 * using with draw arrays (thus indices for a given primitive must be consecutive). */
	GRAPHICS_VERTEX_ARRAY_TYPE_FLOAT_SEPARATE_DRAW_ARRAYS
	/* Other types may support interleaved buffer formats, other value types or
	 * the arbitrary ordering of indices used with draw elements. */
}; /* enum Graphics_vertex_array_type */

/*****************************************************************************//**
 * Specifies the type of the vertices being added or retrieved from the vertex buffer.
 * The formats supported for reading or writing depend on the array type.
 * If the array types supports interleaved values then vertex types for those
 * interleaved values may also be specified.
 * The numbers in the enumerations specify the number of values per vertex. 
 * @see Graphics_vertex_array_type.
*/
enum Graphics_vertex_array_attribute_type
{
	/** Vertex position values. */
	GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_POSITION,
	/** Vertex normal values. */
	GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_NORMAL,
	/** Per vertex colour values. */
	GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_COLOUR,
	/** Per vertex data values. */
	GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_DATA,
	/** First texture coordinate values. */
	GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_TEXTURE_COORDINATE_ZERO,
	/** Specifies that the number of vertices for a primitive. */
	GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_ELEMENT_INDEX_COUNT,
	/** Specifies that the index of the first vertex for a primitive. */
	GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_ELEMENT_INDEX_START,
	/** Records the identifier of a particular primitive for selection and editing. */
	GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_ID
	/* Complex types might be like this...
	 * GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_VERTEX3_NORMAL3
	 * and element_array indices might be supported with an DRAW_ELEMENTS set
	 * type where each vertex index can be unrelated.
	 * GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_ELEMENT_INDEX_3
	 */
}; /* enum Graphics_vertex_array_attribute_type */

/** Private implementation of Graphics_vertex_array */
class Graphics_vertex_array_internal;

/*****************************************************************************//**
 * Object for storing attributes for arrays of vertices.
*/
class Graphics_vertex_array
{
private:
	class Graphics_vertex_array_internal *internal;
	
public:
	
	/*****************************************************************************//**
	 * Construct a new vertex array of the specified type.
	*/
	Graphics_vertex_array(enum Graphics_vertex_array_type type);
	
	/*****************************************************************************//**
	 * Destroys a vertex array.
	*/
	~Graphics_vertex_array();

	/*****************************************************************************//**
	 * Add values to set.
	 * 
	 * @param vertex_type  Specifies the format of the supplied vertices.
	 * @param number_of_values  The size of the values array.
	 * @param values  Array of values, length is required to match that expected by
	 * the specified vertex_type.
	 * @return return_code. 1 for Success, 0 for failure.
	*/
	int add_float_attribute(
			enum Graphics_vertex_array_attribute_type vertex_type,
			unsigned int number_of_values, float *values);

	/*****************************************************************************//**
	 * Retrieve pointer to value buffer from set.
	 * 
	 * @param vertex_buffer_type  Specifies the format expected of the vertex buffer.  If the
	 * type requested is not supported by the set type then the routine will fail.
	 * (This behaviour could be changed to allow format conversion but the point of this
	 * object is to avoid such conversions and so isn't expected normally.)
	 * @param vertex_buffer  Returns a pointer to the vertex buffer.  It is a reference
	 * to the sets own memory and not a copy and so can not be used once the 
	 * set is destroyed or modified and should not be freed.
	 * @param values_per_vertex  Returns the number of values for each vertex.
	 * @param vertex_count  Returns the total number of vertices.  The total number of float
	 * values is the vertex_count * "values per vertex according to vertex type".
	 * @return return_code. 1 for Success, 0 for failure.
	*/
	int get_float_vertex_buffer(
			enum Graphics_vertex_array_attribute_type vertex_buffer_type,
			float **vertex_buffer, unsigned int *values_per_vertex,
			unsigned int *vertex_count);

	/*****************************************************************************//**
	 * Add values to set.
	 * 
	 * @param vertex_type  Specifies the format of the supplied vertices.
	 * @param number_of_values  The size of the values array.
	 * @param values  Array of values, length is required to match that expected by
	 * the specified vertex_type.
	 * @return return_code. 1 for Success, 0 for failure.
	*/
	int add_unsigned_integer_attribute(
			enum Graphics_vertex_array_attribute_type vertex_type,
			unsigned int number_of_values, unsigned int *values);

	/*****************************************************************************//**
	 * Get values from set.
	 * 
	 * @param vertex_type  Specifies the format of the supplied vertices.
	 * @param vertex_index  The index of the vertex that values will be returned for.
	 * @param number_of_values  The expected size of the values array.
	 * @param values  Array of values.
	 * @return return_code. 1 for Success, 0 for failure.
	*/
	int get_unsigned_integer_attribute(
		enum Graphics_vertex_array_attribute_type vertex_type,
		unsigned int vertex_index,	unsigned int number_of_values, unsigned int *values);

	/*****************************************************************************//**
	 * Retrieve pointer to value buffer from set.
	 * 
	 * @param vertex_buffer_type  Specifies the format expected of the vertex buffer.  If the
	 * type requested is not supported by the set type then the routine will fail.
	 * (This behaviour could be changed to allow format conversion but the point of this
	 * object is to avoid such conversions and so isn't expected normally.)
	 * @param vertex_buffer  Returns a pointer to the vertex buffer.  It is a reference
	 * to the sets own memory and not a copy and so can not be used once the 
	 * set is destroyed or modified and should not be freed.
	 * @param vertex_count  Returns the total number of vertices.  The total number of float
	 * values is the vertex_count * "values per vertex according to vertex type".
	 * @return return_code. 1 for Success, 0 for failure.
	*/
	int get_unsigned_integer_vertex_buffer(
		enum Graphics_vertex_array_attribute_type vertex_buffer_type,
		unsigned int **vertex_buffer, unsigned int *values_per_vertex,
		unsigned int *vertex_count);

	/*****************************************************************************//**
	 * Add values to set.
	 * 
	 * @param vertex_type  Specifies the format of the supplied vertices.
	 * @param number_of_values  The size of the values array.
	 * @param values  Array of values.
	 * @return return_code. 1 for Success, 0 for failure.
	*/
	int add_integer_attribute(
		enum Graphics_vertex_array_attribute_type vertex_type,
		unsigned int number_of_values, int *values);

	/*****************************************************************************//**
	 * Get values from set.
	 * 
	 * @param vertex_type  Specifies the format of the supplied vertices.
	 * @param number_of_values  The expected size of the values array.
	 * @param vertex_index  The index of the vertex that values will be returned for.
	 * @param values  Array of values.
	 * @return return_code. 1 for Success, 0 for failure.
	*/
	int get_integer_attribute(
		enum Graphics_vertex_array_attribute_type vertex_type,
		unsigned int vertex_index,	unsigned int number_of_values, int *values);

	/*****************************************************************************//**
	 * Gets the current size of specified buffer.
	 * 
	 * @param vertex_buffer_type  Specifies that the size should be for buffer of this type.
	 * This actual buffers created and used for different attributes depends on the array type.
	 * @return buffer size.
	*/
	unsigned int get_number_of_vertices(
		enum Graphics_vertex_array_attribute_type vertex_type);

	/*****************************************************************************//**
	 * Resets the sizes of all the buffers in the set.  Does not actually
	 * release memory in the buffers as it is assumed likely that the same buffers
	 * will be recreated.
	 * 
	 * @return return_code.
	*/
	int clear_buffers();

};

typedef int (*Graphics_object_glyph_labels_function)(Triple coordinate_scaling,
	int label_bounds_dimension, int label_bounds_components, float *label_bounds,
	struct Graphical_material *material, struct Graphical_material *secondary_material,
	struct Graphics_font *font, Render_graphics *renderer);
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
Used for rendering a per compile custom addon to a glyph, such as a grid or tick
marks showing the scale of the glyph.
<coordinate_scaling> gives a representative size for determining the number of 
ticks.
==============================================================================*/

Graphics_object_glyph_labels_function Graphics_object_get_glyph_labels_function(
	struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 19 September 2005

DESCRIPTION :
Gets the glyph_labels_function of the <graphics_object>.
This function enables a custom, per compile, labelling for a graphics object 
==============================================================================*/

int Graphics_object_set_glyph_labels_function(struct GT_object *graphics_object,
	Graphics_object_glyph_labels_function glyph_labels_function);
/*******************************************************************************
LAST MODIFIED : 19 September 2005

DESCRIPTION :
Sets the glyph_labels_function of the <graphics_object>.
This function enables a custom, per compile, labelling for a graphics object 
==============================================================================*/

Graphics_object_glyph_labels_function Graphics_object_get_glyph_labels_function(
	struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 19 September 2005

DESCRIPTION :
Gets the glyph_labels_function of the <graphics_object>.
This function enables a custom, per compile, labelling for a graphics object 
==============================================================================*/

#endif /* GRAPHICS_OBJECT_HPP */
