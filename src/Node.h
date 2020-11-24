#pragma once

#include <chrono>
#include <memory>
#include <random>
#include <vector>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/steady_timer.hpp>

namespace gossip {
namespace simulator {

class Node final : public std::enable_shared_from_this<Node> {
private:
    struct Tag{};

public:
    struct Stats {
        std::chrono::time_point<std::chrono::system_clock> firstReceived;
        int numReceived{ 0 };
        int numSent{ 0 };
    };

    Node(boost::asio::io_context& io,
         uint16_t udpPort,
         std::vector<uint16_t> neighbors,
         std::chrono::milliseconds period,
         int fanout,
         Tag);

    static std::shared_ptr<Node> create(boost::asio::io_context& io,
                                        uint16_t udpPort,
                                        std::vector<uint16_t> neighbors,
                                        std::chrono::milliseconds period,
                                        int fanout);

    uint16_t port() const;
    const std::vector<uint16_t>& neighbors() const;
    const Stats& stats() const;

private:
    void start_();
    void receive_();
    void sendLoop_();
    void prepareSend_();
    void sendNext_(std::vector<uint16_t> neighbors);

    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint peer_;
    boost::asio::steady_timer timer_;
    std::string buf_;
    std::string msg_;
    std::vector<uint16_t> neighbors_;
    std::chrono::milliseconds period_{ 5000 };
    int fanout_{ 1 };
    std::default_random_engine rand_;
    Stats stats_;
};

} // namespace simulator
} // namespace gossip

