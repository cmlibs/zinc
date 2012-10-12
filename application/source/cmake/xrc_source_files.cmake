
# Define XRC_SRCS
 SET( XRC_SRCS source/comfile/comfile_window_wx.xrc
	source/command/about_box.xrc source/command/command_window.xrc
	source/element/element_point_tool.xrc source/element/element_point_viewer_wx.xrc
	source/element/element_tool.xrc source/graphics/graphics_window.xrc
	source/graphics/region_tree_viewer_wx.xrc source/graphics/spectrum_editor_wx.xrc
	source/graphics/transform_tool.xrc source/material/material_editor_wx.xrc
	source/node/node_tool.xrc source/node/node_viewer_wx.xrc )
IF( USE_OPENCASCADE )
	SET( XRC_SRCS ${XRC_SRCS} source/cad/cad_tool.xrc )
ENDIF( USE_OPENCASCADE )
