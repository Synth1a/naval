#pragma once

template <typename FuncType>
__forceinline static FuncType call_virtual(void* ppClass, int index)
{
	int* pVTable = *(int**) ppClass; //-V206
	int dwAddress = pVTable[index]; //-V108
	return (FuncType)(dwAddress);
}

template <typename t>
static t get_virtual(void* class_pointer, size_t index) {
	return (*(t**)class_pointer)[index];
	return (*(t**)class_pointer)[index];
	return (*(t**)class_pointer)[index];
}