
# Defines API_HDRS
SET( API_HDRS
    source/api/zinc/context.h
    source/api/zinc/core.h
    source/api/zinc/differentialoperator.h
    source/api/zinc/element.h
    source/api/zinc/field.h
    source/api/zinc/fieldalias.h
    source/api/zinc/fieldarithmeticoperators.h
    source/api/zinc/fieldcomposite.h
    source/api/zinc/fieldconditional.h
    source/api/zinc/fieldconstant.h
    source/api/zinc/fieldcoordinatetransformation.h
    source/api/zinc/fieldfiniteelement.h
    source/api/zinc/fieldgroup.h
    source/api/zinc/fieldimage.h
    source/api/zinc/fieldimageprocessing.h
    source/api/zinc/fieldlogicaloperators.h
    source/api/zinc/fieldmatrixoperators.h
    source/api/zinc/fieldmodule.h
    source/api/zinc/fieldnodesetoperators.h
    source/api/zinc/fieldsceneviewerprojection.h
    source/api/zinc/fieldsubobjectgroup.h
    source/api/zinc/fieldtime.h
    source/api/zinc/fieldtrigonometry.h
    source/api/zinc/fieldvectoroperators.h
    source/api/zinc/graphic.h
    source/api/zinc/graphicsfilter.h
    source/api/zinc/graphicsfont.h
    source/api/zinc/graphicsmaterial.h
    source/api/zinc/graphicsmodule.h
    source/api/zinc/interactivetool.h
    source/api/zinc/node.h
    source/api/zinc/optimisation.h
    source/api/zinc/region.h
    source/api/zinc/rendition.h
    source/api/zinc/scene.h
    source/api/zinc/selection.h
    source/api/zinc/spectrum.h
    source/api/zinc/status.h
    source/api/zinc/stream.h
    source/api/zinc/tessellation.h
    source/api/zinc/time.h
    source/api/zinc/timekeeper.h
    source/api/zinc/timesequence.h
    source/api/zinc/types/contextid.h
    source/api/zinc/types/differentialoperatorid.h
    source/api/zinc/types/elementid.h
    source/api/zinc/types/fieldaliasid.h
    source/api/zinc/types/fieldfiniteelementid.h
    source/api/zinc/types/fieldgroupid.h
    source/api/zinc/types/fieldid.h
    source/api/zinc/types/fieldimageid.h
    source/api/zinc/types/fieldimageprocessingid.h
    source/api/zinc/types/fieldmoduleid.h
    source/api/zinc/types/fieldsubobjectgroupid.h
    source/api/zinc/types/graphicid.h
    source/api/zinc/types/graphicscoordinatesystem.h
    source/api/zinc/types/graphicsfilterid.h
    source/api/zinc/types/graphicsmaterialid.h
    source/api/zinc/types/graphicsmoduleid.h
    source/api/zinc/types/graphicsrendertype.h
    source/api/zinc/types/interactivetoolid.h
    source/api/zinc/types/nodeid.h
    source/api/zinc/types/optimisationid.h
    source/api/zinc/types/regionid.h
    source/api/zinc/types/renditionid.h
    source/api/zinc/types/sceneid.h
    source/api/zinc/types/selectionid.h
    source/api/zinc/types/spectrumid.h
    source/api/zinc/types/streamid.h
    source/api/zinc/types/tessellationid.h
    source/api/zinc/types/timeid.h
    source/api/zinc/types/timekeeperid.h
    source/api/zinc/types/timesequenceid.h )

IF( USE_OPENCASCADE )
    SET( API_HDRS ${API_HDRS}
        source/api/zinc/fieldcad.h
        source/api/zinc/types/fieldcadid.h )
ENDIF( USE_OPENCASCADE )

IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
    SET( API_HDRS ${API_HDRS}
        source/api/zinc/sceneviewer.h
        source/api/zinc/types/sceneviewerid.h )

    SET( NEW_API_HDRS ${NEW_API_HDRS}
      source/api/zinc/sceneviewerinput.h )

ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )

SET( API_HDRS ${API_HDRS} ${NEW_API_HDRS} )
