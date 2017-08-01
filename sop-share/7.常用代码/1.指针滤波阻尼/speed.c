#define SPEED_DRAW_INTERVAL 6
#define TACH_DRAW_INTERVAL 8

#define SPEED_DELTA				(0.5 * 1.0/240.0)
#define TACH_DELTA				(0.1 * 0.5/(8*2))
#define SPEED_DAMPING_FACTOR	(16)
#define TACH_DAMPING_FACTOR		(32)

/*
#define DampingEffectionImpl(TargetQueue, TargetQueueLock, TargetValue, OldValue, ActualValue, DAMPING_FACTOR, DELTA) \
    do { \
        static kzBool bIterateContinue = KZ_FALSE; \
        if(TargetQueue.empty() != KZ_TRUE) \
        { \
            result = kzsThreadLockAcquire(TargetQueueLock); \
            kzThrowIfError(result); \
            BaseMsgSharedPtr msg = TargetQueue.front(); \
            TargetQueue.pop(); \
            result = kzsThreadLockRelease(TargetQueueLock); \
            kzThrowIfError(result); \
            CDataForKanziAPP::Instance().TargetValue = msg->GetData(); \
            ActualValue = CDataForKanziAPP::Instance().OldValue; \
            bIterateContinue = KZ_TRUE; \
        } else {  \
            if (bIterateContinue == KZ_TRUE) {  \
                ActualValue = CDataForKanziAPP::Instance().OldValue + (CDataForKanziAPP::Instance().TargetValue - CDataForKanziAPP::Instance().OldValue) / DAMPING_FACTOR;\
                CDataForKanziAPP::Instance().OldValue = ActualValue; \
                \
                if ((CDataForKanziAPP::Instance().OldValue - CDataForKanziAPP::Instance().TargetValue) / (float)CDataForKanziAPP::Instance().TargetValue < (DELTA) && \
                    (CDataForKanziAPP::Instance().OldValue - CDataForKanziAPP::Instance().TargetValue) / (float)CDataForKanziAPP::Instance().TargetValue > (-DELTA)) { \
                    bIterateContinue = KZ_FALSE; \
                }  \
            }  \
            else {  \
                ActualValue = CDataForKanziAPP::Instance().TargetValue;  \
            }  \
        }  \
    } while(0)
*/

#include <stdlib.h>

struct damping_effection_info{
	int current;
	int taget;
	int delta;
	int factor;
};

#define INIT_SPEED_DAMPING() {\
	.current = 0,\
	.target  = 0,\
	.delta   = SPEED_DELTA,\
	.factor  = SPEED_DAMPING_FACTOR\
}

#define INIT_TACH_DAMPING() {\
	.current = 0,\
	.target  = 0,\
	.delta   = TACH_DELTA,\
	.factor  = TACH_DAMPING_FACTOR\
}

    
/*
 *@return 1:
 *@return 0:
 */
inline int do_damping_effection(int* current, int target, int delta, int factor)
{
	if( abs(target - (*current) ) < delta ){
		*current = target;
		return 0;
	}
	
	*current += ((target - *current) / factor);
	return 1;
}

int damping_effection_impl(struct damping_effection_info* info)
{
	return do_damping_effection( &(info->current),
							    info->target,
								info->delta,
								info->factor);
}

inline void damping_effection_set_target(struct damping_effection_info* info, int target)
{
	info->target = target;
}

inline int damping_effection_get_current(struct damping_effection_info* info)
{
	return info->current;
}
