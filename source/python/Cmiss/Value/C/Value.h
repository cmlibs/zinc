#ifndef PY_CMISS_VALUE_H
#define PY_CMISS_VALUE_H
#ifdef __cplusplus
extern "C" {
#endif

extern DL_IMPORT(PyTypeObject) CmissValueType;

#define CmissValue_Check(op) PyObject_TypeCheck(op, &CmissValueType)
#define CmissValue_CheckExact(op) ((op)->ob_type == &CmissValueType)

#ifdef __cplusplus
}
#endif
#endif /* !PY_CMISS_VALUE_H */
