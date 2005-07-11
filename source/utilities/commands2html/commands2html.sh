#! /bin/tcsh -f

set CMGUI = $1
set AWK_SCRIPT = ${CMISS_ROOT}/cmgui/source/utilities/commands2html/commands2html.awk
set HTML_TEMPLATE = ${CMISS_ROOT}/cmgui/source/utilities/commands2html/template.html

${CMGUI} -command_list | awk -v TEMPLATE=${HTML_TEMPLATE} -f ${AWK_SCRIPT}
