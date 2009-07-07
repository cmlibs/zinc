
# Defines UNEMAP_SRCS

IF( USE_UNEMAP )

	SET( UNEMAP TRUE )
	SET( SPECTRAL_TOOLS TRUE )
	SET( UNEMAP_USE_3D TRUE )
	SET( NOT_ACQUISITION_ONLY TRUE )

	SET( UNEMAP_SRCS unemap_application/acquisition.c
		unemap_application/acquisition_window.c
		unemap_application/acquisition_work_area.c
		unemap_application/analysis.c
		unemap_application/analysis_calculate.c
		unemap_application/analysis_drawing.c
		unemap_application/analysis_window.c
		unemap_application/analysis_work_area.c
		unemap_application/bard.c
		unemap_application/beekeeper.c
		unemap_application/cardiomapp.c
		unemap_application/delaunay.c
		unemap_application/edf.c
		unemap_application/eimaging_time_dialog.c
		unemap_application/drawing_2d.c
		unemap_application/interpolate.c
		unemap_application/map_dialog.c
		unemap_application/mapping.c
		unemap_application/mapping_window.c
		unemap_application/neurosoft.c
		unemap_application/pacing_window.c
		unemap_application/page_window.c
		unemap_application/rig.c
		unemap_application/rig_node.c
		unemap_application/setup_dialog.c
		unemap_application/spectral_methods.c
		unemap_application/system_window.c
		unemap_application/trace_window.c
		unemap_application/unemap_command.c
		unemap_application/unemap_hardware_client.c
		unemap_application/unemap_package.c )
ENDIF( USE_UNEMAP )

