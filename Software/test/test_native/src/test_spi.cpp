#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../include/spi_mock.hpp"

#include <iostream>

using ::testing::_;
using ::testing::Return;
using namespace std;

#include "../../../src/SPI.cpp"
SPI *SPIObj;

namespace SPITests
{
// System under test


    class SPIAPIsTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            // Set up mock object
            SPIMockObj = new SPIMock;
            SPIObj = new SPI;
        }

        void TearDown() override
        {
            // Clean up the mock object used
            delete SPIMockObj;
        }
    };

    TEST_F(SPIAPIsTest, TestConfigureSPI)
    {
        // List of tests to be done for function TestConfigureGPIO
        // 1 - Test returning success from gpio_config and gpio_set_level
        // 2 - Test returning error from gpio_config
        // 3 - Test returning error from gpio_set_level

        bool TestHasError = 0;
        bool test_return = false;
        spi_host_device_t mock_host_id = 0;
        spi_bus_config_t *mock_bus_config = NULL; 
        spi_dma_chan_t mock_dma_chan = 0;
        spi_device_interface_config_t *mock_dev_config = NULL;
        spi_device_handle_t *mock_handle = NULL;

        //////////////////////////////////////////////// Test 1 /////////////////////////////////////////////////////////////////////////////////////
        //cout << endl
        //     << "[   INFO   ] Test 1 - Test returning success from gpio_config and gpio_set_level" << endl;
        //GTEST_LOG_(INFO) << "Test 1 - Test returning success from gpio_config and gpio_set_level";
        printf("[   INFO   ] Test 1 - Test returning success from gpio_config and gpio_set_level\n");

        EXPECT_CALL(*SPIMockObj, spi_bus_initialize(mock_host_id, testing::_, mock_dma_chan)) // The called argument is a pointer, so we will test call expectation with the any argument pararameter
            .Times(1)
            .WillOnce(Return(ESP_OK));

        //EXPECT_CALL(*SPIMockObj, spi_bus_add_device(testing::_, testing::_, testing::_))
        //    .Times(1)
        //    .WillOnce(Return(ESP_OK));

        test_return = SPIObj->Begin(mock_bus_config);
        //EXPECT_EQ(true, test_return);
        EXPECT_EQ(true, test_return) << (TestHasError = true); // If expectation fails, set TestHasError to true;
        if (TestHasError)
            std::cout << "[   FAIL   ] " << "Test 1 fail" << endl;
        else
            std::cout << "[    OK    ] " << "Test 1 pass" << endl;
    }

    TEST_F(SPIAPIsTest, Test)
    {
        ASSERT_TRUE(false);
    }
}