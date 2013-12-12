#include "gtest/gtest.h"
#include "storm-config.h"
#include "src/exceptions/InvalidArgumentException.h"
#include "src/storage/PartialScheduler.h"
#include "src/storage/TotalScheduler.h"

TEST(SchedulerTest, PartialScheduler) {
    storm::storage::PartialScheduler scheduler;
    
    ASSERT_NO_THROW(scheduler.setChoice(0, 1));
    ASSERT_NO_THROW(scheduler.setChoice(0, 3));
    ASSERT_NO_THROW(scheduler.setChoice(3, 4));
    
    ASSERT_TRUE(scheduler.isChoiceDefined(0));
    ASSERT_EQ(3, scheduler.getChoice(0));
    
    ASSERT_TRUE(scheduler.isChoiceDefined(3));
    ASSERT_EQ(4, scheduler.getChoice(3));
    
    ASSERT_FALSE(scheduler.isChoiceDefined(1));
    ASSERT_THROW(scheduler.getChoice(1), storm::exceptions::InvalidArgumentException);
}

TEST(SchedulerTest, TotalScheduler) {
    storm::storage::TotalScheduler scheduler(4);
    
    ASSERT_NO_THROW(scheduler.setChoice(0, 1));
    ASSERT_NO_THROW(scheduler.setChoice(0, 3));
    ASSERT_NO_THROW(scheduler.setChoice(3, 4));
    
    ASSERT_TRUE(scheduler.isChoiceDefined(0));
    ASSERT_EQ(3, scheduler.getChoice(0));
    
    ASSERT_TRUE(scheduler.isChoiceDefined(3));
    ASSERT_EQ(4, scheduler.getChoice(3));
    
    ASSERT_TRUE(scheduler.isChoiceDefined(1));
    ASSERT_EQ(0, scheduler.getChoice(1));
                                             
    ASSERT_THROW(scheduler.getChoice(4), storm::exceptions::InvalidArgumentException);
    ASSERT_THROW(scheduler.setChoice(5, 2), storm::exceptions::InvalidArgumentException);
}