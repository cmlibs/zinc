
FUNCTION( WXWIDGETS_SRCS _OUTFILES )
	SET( XRC_SRCS source/comfile/comfile_window_wx.xrc
		source/command/about_box.xrc source/command/command_window.xrc
		source/element/element_point_tool.xrc source/element/element_point_viewer_wx.xrc
		source/element/element_tool.xrc source/graphics/graphics_window.xrc
		source/graphics/scene_editor_wx.xrc source/graphics/spectrum_editor_wx.xrc
		source/graphics/transform_tool.xrc source/material/material_editor_wx.xrc
		source/node/node_tool.xrc source/node/node_viewer_wx.xrc )

	FOREACH( XRC_SRC ${XRC_SRCS} )
		STRING( REGEX MATCH "/([a-z_A-Z]*)\\.xrc$" DUMMY "${XRC_SRC}" )
		SET( XRC_FCN ${CMAKE_MATCH_1} )
		WXWIDGETS_ADD_RESOURCES( XRC_OUTPUT ${XRC_SRC} OPTIONS --cpp-code
			--function=wxXmlInit_${XRC_FCN} --output=${PROJECT_BINARY_DIR}/${XRC_SRC}h )
		#MESSAGE( STATUS ${XRC_FCN} )
	ENDFOREACH( XRC_SRC ${XRC_SRCS} )
	# --output=${PROJECT_BINARY_DIR}/source/element )
	FILE( MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/source/comfile )
	FILE( MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/source/command )
	FILE( MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/source/element )
	FILE( MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/source/graphics )
	FILE( MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/source/material )
	FILE( MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/source/node )

	SET( ${_OUTFILES} ${XRC_OUTPUT} PARENT_SCOPE )
ENDFUNCTION( WXWIDGETS_SRCS _OUTFILES )

