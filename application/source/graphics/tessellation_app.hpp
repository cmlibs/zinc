
struct Option_table;
struct Parse_state;

/***************************************************************************//**
 * Adds a token to the option_table which if matched reads the following string
 * as a series of positive integers separated by *, mainly used for element
 * discretization.
 *
 * @param option_table  The command option table to add the entry to.
 * @param token  The required token for this option.
 * @param divisions_address  Address of where to allocate return int array.
 * Must be initialised to NULL or allocated array.
 * @param size_address  Address of int to receive array size. Must be
 * initialised to size of initial *divisions_address, or 0 if none.
 */
int Option_table_add_divisions_entry(struct Option_table *option_table,
	const char *token, int **divisions_address, int *size_address);

/***************************************************************************//**
 * Adds an entry to the <option_table> under the given <token> that selects a
 * Cmiss_tessellation from the graphics_module.
 * @param tessellation_address  Address of tessellation pointer which must be
 * NULL or ACCESSed.
 */
int Option_table_add_Cmiss_tessellation_entry(struct Option_table *option_table,
	const char *token, struct Cmiss_graphics_module *graphics_module,
	struct Cmiss_tessellation **tessellation_address);

/***************************************************************************//**
 * gfx define tessellation command.
 * @param state  Command parse state.
 * @param graphics_module_void  Cmiss_graphics_module.
 */
int gfx_define_tessellation(struct Parse_state *state, void *dummy_to_be_modified,
	void *graphics_module_void);

/***************************************************************************//**
 * gfx destroy tessellation command.
 * @param state  Command parse state.
 * @param graphics_module_void  Cmiss_graphics_module.
 */
int gfx_destroy_tessellation(struct Parse_state *state, void *dummy_to_be_modified,
	void *graphics_module_void);

/***************************************************************************//**
 * gfx list tessellation command.
 */
int gfx_list_tessellation(struct Parse_state *state, void *dummy_to_be_modified,
	void *graphics_module_void);

