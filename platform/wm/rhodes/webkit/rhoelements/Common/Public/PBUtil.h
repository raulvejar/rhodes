/** Declaration if type for exported functions from DLL */
#define EXPORT_TYPE extern "C" __declspec(dllexport)

/** Macro giving count of elements in an array */
#define COUNTOF(x) (sizeof(x)/sizeof(x[0]))

/** Macro to copy a wide string, guaranteed not to overrun the destination and always to be null terminated */
#define WSAFECOPY(d,s)	StringCchCopy(d,COUNTOF(d),s)

/** Macro to write to PBModule log automatically adding function name and line number */
#define LOG(s,m) Log(s, m,_T(__FUNCTION__), __LINE__)
