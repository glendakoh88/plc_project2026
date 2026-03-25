/*use pragma pack to ensure compiler does not add padding*/
#pragma pack(push, 1)
typedef struct{
    uint8_t B;
    uint8_t M;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} BMPHeader;
typedef struct {
    uint32_t header_size;     
    int32_t width;
    int32_t height;
    uint16_t planes;          
    uint16_t bits;           
    uint32_t compression;     
    uint32_t image_size;      
    int32_t x_resolution;
    int32_t y_resolution;
    uint32_t colors_used;    
    uint32_t important_colors;
} DIBHeader;
#pragma pack(pop)

