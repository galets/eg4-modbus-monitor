#pragma once

#include <modbus/modbus.h>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <vector>

enum class RegisterType {
    INPUT,
    HOLDING
};

class ModbusInterface {
public:
    virtual ~ModbusInterface() = default;
    virtual bool isValid() const = 0;
    virtual std::vector<uint16_t> readRegisters(RegisterType type, int start_address, int num_registers) const = 0;
    virtual void writeRegisters(int start_address, const std::vector<uint16_t>& values) const = 0;
};

class Modbus : public ModbusInterface
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
    Modbus(const std::string& device, int baud, char parity, int data_bits, int stop_bits, int slave_id)
        : ctx_(modbus_new_rtu(device.c_str(), baud, parity, data_bits, stop_bits))
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
    Modbus(const std::string& ip_address, int port, int slave_id)
        : ctx_(modbus_new_tcp(ip_address.c_str(), port))
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
    ~Modbus()
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
    std::vector<uint16_t> readRegisters(RegisterType type, int start_address, int num_registers) const
    {
        checkValid();

        std::vector<uint16_t> registers(num_registers);
        int rc;
        bool reset_tried = false;
        const int MAXRETRIES = 3;
        for (int retry = 0; retry < MAXRETRIES; ++retry) {
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

            if (rc >= 0) break; // Success

            if (errno != 110 && errno != EMBBADCRC) break; // Non-retryable error

            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // Flush the input buffer to clear any stale data
            modbus_flush(ctx_);
            
            // After last retry, try resetting the connection once
            if (retry == MAXRETRIES - 1 && !reset_tried) {
                modbus_close(ctx_);
                if (modbus_connect(ctx_) == -1) {
                    throw std::runtime_error("Failed to reconnect to Modbus device");
                }
                reset_tried = true;
                retry = -1; // Reset retry counter
            }
        }

        if (rc < 0)
        {
            throw std::runtime_error("Failed to read registers, error: " + std::to_string(errno));
        }
        return registers;
    }


    /**
     * @brief Write multiple registers to the Modbus device.
     *
     * @param start_address The starting register address.
     * @param values The values to write.
     * @throws std::runtime_error if there's an error during the write operation.
     */
    void writeRegisters(int start_address, const std::vector<uint16_t>& values) const {
        checkValid();

        // Notice: modbus_write_registers fails, must write them one at a time
        for (auto i = 0; i < values.size(); ++i) {
            int rc;
            for (int retry = 0; retry < 3; ++retry) {
                rc = modbus_write_register(ctx_, start_address + i, values.at(i));
                if (rc >= 0 || errno != 110) {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            if (rc < 0)
            {
                throw std::runtime_error("Failed to write registers, error: " + std::to_string(errno));
            }
        }
    }

    /**
     * @brief Check if the connection is valid.
     *
     * @return true If the connection is valid.
     * @return false otherwise
     */
    bool isValid() const
    {
        return ctx_ != nullptr;
    }

private:
    modbus_t* ctx_;

    void checkValid() const
    {
        if (ctx_ == nullptr)
        {
            throw std::runtime_error("Modbus context is not valid");
        }
    }
};

class SerializedModbus : public ModbusInterface {
private:
    ModbusInterface& modbus_;
    mutable std::mutex mutex_;
    mutable std::condition_variable write_cv_;
    mutable bool writeHappened_;

public:
    explicit SerializedModbus(ModbusInterface& modbus)
        : modbus_(modbus) {
    }

    ~SerializedModbus() = default;
    SerializedModbus(const SerializedModbus&) = delete;
    SerializedModbus& operator=(const SerializedModbus&) = delete;
    SerializedModbus(SerializedModbus&&) = delete;
    SerializedModbus& operator=(SerializedModbus&&) = delete;

    bool isValid() const {
        return modbus_.isValid();
    }

    std::vector<uint16_t> readRegisters(RegisterType type, int start_address, int num_registers) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return modbus_.readRegisters(type, start_address, num_registers);
    }

    void writeRegisters(int start_address, const std::vector<uint16_t>& values) const {
        std::lock_guard<std::mutex> lock(mutex_);
        modbus_.writeRegisters(start_address, values);
        writeHappened_ = true;
        write_cv_.notify_all();
    }

    void wait(uint32_t millisec) {
        std::unique_lock<std::mutex> lock(mutex_);
        writeHappened_ = false;
        write_cv_.wait_for(lock, std::chrono::milliseconds(millisec), [this] { return writeHappened_; });
    }
};
