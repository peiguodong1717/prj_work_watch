#include "sys.h"



uint8_t g_stm32_uniqueId[STM32_UNIQUE_ID_SIZE] = {0};

//////////////////////////////////////////////////////////////////////////////////	 
//********************************************************************************  
void NVIC_Configuration(void)
{

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�

}


void util_readStm32UniqueCode(void) 
{
    vu8 * addr = (vu8*)(0x1FFFF7E8);
    u8 i;
    for(i=0; i<STM32_UNIQUE_ID_SIZE; ++i) {
        u8 v = *addr;
        g_stm32_uniqueId[i] = v;
        ++addr;
    }
}
