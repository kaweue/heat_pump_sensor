#ifdef ARDUINO
#include <pgmspace.h>
#endif
#define LABELDEF

class LabelDef
{
public:
    enum class DataType : int
    {
        UNKNOWN = -1,
        TEMPERATURE = 1,
        PRESSURE = 2,
        VOLTAGE = 3,
        CURRENT = 4,
        FLOW = 5,
        DATA_SIZE = 6,
        SIGNAL_STRENGTH = 7,
    };

    int convid;
    int offset;
    int registryID;
    int dataSize;
    DataType dataType;
    const char *label;
    char *data;
    char asString[30];
    LabelDef() {};
    LabelDef(int registryIDp, int offsetp, int convidp, int dataSizep, DataType dataTypep, const char *labelp) : convid(convidp), offset(offsetp), registryID(registryIDp), dataSize(dataSizep), dataType(dataTypep), label(labelp) {};
};
