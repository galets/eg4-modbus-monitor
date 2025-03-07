#pragma once

#include <modbus/modbus.h>
#include <iostream>
#include <stdexcept>
#include <vector>

enum class RegisterType {
    INPUT,
    HOLDING
};

class ModbusReader
{
public:
    /**
     * @brief Construct a new Modbus Reader object for RTU mode.
     *
     * @param device The serial device path (e.g., "/dev/ttyUSB0").
     * @param baud The baud rate (e.g., 19200).
     * @param parity The parity ('N', 'E', or 'O').
     * @param data_bits The number of data bits (e.g., 8).
     * @param stop_bits The number of stop bits (e.g., 1).
     * @param slave_id The Modbus slave ID.
     * @throws std::runtime_error if there's an error during initialization.
     */
    ModbusReader(const char *device, int baud, char parity, int data_bits, int stop_bits, int slave_id)
        : ctx_(modbus_new_rtu(device, baud, parity, data_bits, stop_bits))
    {
        if (ctx_ == nullptr)
        {
            throw std::runtime_error("Failed to create Modbus RTU context");
        }
        modbus_set_slave(ctx_, slave_id);
        if (modbus_connect(ctx_) == -1)
        {
            modbus_free(ctx_);
            ctx_ = nullptr;
            throw std::runtime_error("Failed to connect to Modbus RTU device");
        }
    }

    /**
     * @brief Construct a new Modbus Reader object for TCP mode.
     *
     * @param ip_address The IP address of the Modbus server.
     * @param port The port number (usually 502).
     * @param slave_id The Modbus slave ID.
     * @throws std::runtime_error if there's an error during initialization.
     */
    ModbusReader(const char *ip_address, int port, int slave_id)
        : ctx_(modbus_new_tcp(ip_address, port))
    {
        if (ctx_ == nullptr)
        {
            throw std::runtime_error("Failed to create Modbus TCP context");
        }
        modbus_set_slave(ctx_, slave_id);

        if (modbus_connect(ctx_) == -1)
        {
            modbus_free(ctx_);
            ctx_ = nullptr;
            throw std::runtime_error("Failed to connect to Modbus TCP server");
        }
    }

    /**
     * @brief Destroy the Modbus Reader object.
     *        Closes the Modbus connection and frees the context.
     */
    ~ModbusReader()
    {
        if (ctx_ != nullptr)
        {
            modbus_close(ctx_);
            modbus_free(ctx_);
        }
    }

    /**
     * @brief Read multiple registers from the Modbus device.
     *
     * @param start_address The starting register address.
     * @param num_registers The number of registers to read.
     * @return std::vector<uint16_t> A vector containing the register values.
     * @throws std::runtime_error if there's an error during the read operation.
     */
    std::vector<uint16_t> readRegisters(RegisterType type, int start_address, int num_registers)
    {
        checkValid();

        std::vector<uint16_t> registers(num_registers);
        int rc;
        switch (type) {
            case RegisterType::INPUT:
                rc = modbus_read_input_registers(ctx_, start_address, num_registers, registers.data());
                break;
            case RegisterType::HOLDING:
                rc = modbus_read_registers(ctx_, start_address, num_registers, registers.data());
                break;
            default:
                throw std::runtime_error("Invalid register type");
        }

        if (rc < 0)
        {
            throw std::runtime_error("Failed to read registers");
        }
        return registers;
    }

    /**
     * @brief Check if the connection is valid.
     *
     * @return true If the connection is valid.
     * @return false otherwise
     */
    bool isValid()
    {
        return ctx_ != nullptr;
    }

private:
    modbus_t *ctx_;

    void checkValid()
    {
        if (ctx_ == nullptr)
        {
            throw std::runtime_error("Modbus context is not valid");
        }
    }
};
