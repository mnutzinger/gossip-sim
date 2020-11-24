#include <iostream>
#include <string>

#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/address_v6.hpp>
#include <boost/system/error_code.hpp>

#include <range/v3/action/shuffle.hpp>
#include <range/v3/action/take.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/cache1.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/transform.hpp>

#include "Node.h"

using std::chrono::milliseconds;
using std::chrono::system_clock;
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::vector;

using boost::asio::buffer;
using boost::asio::io_context;
using boost::asio::ip::address_v6;
using boost::asio::ip::udp;
using boost::system::error_code;

using ranges::to;

namespace actions = ranges::actions;
namespace views = ranges::views;

namespace gossip {
namespace simulator {

namespace {

constexpr size_t maxReceiveBytes{ 1024 };

string toString(const vector<uint16_t>& nums)
{
    string s = nums | views::transform([](uint16_t num) {
        return std::to_string(num);
    }) | views::cache1 | views::join(" ") | to<string>;

    return "[ " + s + " ]";
}

} // namespace

Node::Node(io_context& io,
           uint16_t udpPort,
           vector<uint16_t> neighbors,
           milliseconds period,
           int fanout,
           Tag) :
    socket_(io, udp::endpoint(udp::v6(), udpPort)),
    timer_(io),
    neighbors_(std::move(neighbors)),
    period_(std::move(period)),
    fanout_(fanout),
    rand_(system_clock::now().time_since_epoch().count())
{
    buf_.resize(maxReceiveBytes);
    std::cout << port() << " started, neighbors=" << toString(neighbors_) <<
        ", period=" << period_.count() << "ms, fanout=" << fanout_ << std::endl;
}

shared_ptr<Node> Node::create(io_context& io,
                              uint16_t udpPort,
                              vector<uint16_t> neighbors,
                              milliseconds period,
                              int fanout)
{
    auto node = make_shared<Node>(io,
                                  udpPort,
                                  std::move(neighbors),
                                  std::move(period),
                                  fanout,
                                  Tag{});
    node->start_();
    return node;
}

uint16_t Node::port() const
{
    return socket_.local_endpoint().port();
}

const vector<uint16_t>& Node::neighbors() const
{
    return neighbors_;
}

const Node::Stats& Node::stats() const
{
    return stats_;
}

void Node::start_()
{
    receive_();
    sendLoop_();
}

void Node::receive_()
{
    socket_.async_receive_from(
        buffer(buf_),
        peer_,
        [this, keep=shared_from_this()](const error_code& err, size_t num) {
        if (err) {
            std::cerr << port() << " async_receive_from: " << err.message() << std::endl;
            return;
        }

        if (msg_.empty()) {
            stats_.firstReceived = system_clock::now();
        }
        ++stats_.numReceived;

        msg_ = buf_.substr(0, num);
        receive_();
    });
}

void Node::sendLoop_()
{
    timer_.expires_after(period_);
    timer_.async_wait(
        [this, keep=shared_from_this()](const error_code& err) {
        if (err) {
            std::cerr << port() << " async_wait: " << err.message() << std::endl;
            return;
        }

        prepareSend_();
        sendLoop_();
    });
}

void Node::prepareSend_()
{
    if (msg_.empty()) {
        return;
    }

    vector<uint16_t> neighbors = neighbors_;
    neighbors |= actions::shuffle(rand_) | actions::take(fanout_);
    std::cout << port() << " fanout to " << toString(neighbors) << std::endl;
    sendNext_(std::move(neighbors));
    ++stats_.numSent;
}

void Node::sendNext_(vector<uint16_t> neighbors)
{
    if (neighbors.empty()) {
        return;
    }

    uint16_t neighbor = neighbors.back();
    neighbors.pop_back();

    socket_.async_send_to(
        buffer(msg_),
        udp::endpoint(address_v6::loopback(), neighbor),
        [this,
         keep=shared_from_this(),
         neighbor,
         neighbors=std::move(neighbors)](const error_code& err, size_t num) {
        if (err) {
            std::cerr << port() << " async_send_to(" << neighbor << "): " <<
                err.message() << std::endl;
            return;
        }

        sendNext_(std::move(neighbors));
    });
}

} // namespace simulator
} // namespace gossip

