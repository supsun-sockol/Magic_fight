#ifndef CLIENT_APP_NETCONNECTION_HPP
#define CLIENT_APP_NETCONNECTION_HPP

#include "NetCommon.hpp"
#include "NetThreadSafeQueue.hpp"

namespace net
{
    class connection : public std::enable_shared_from_this<connection>
    {
    public:
        enum class owner
        {
            server,
            client
        };

        connection(owner parent, boost::asio::io_context& context,
                   boost::asio::ip::tcp::socket socket, tsqueue<owned_message>& in_queue) :
                   _context(context), _socket(std::move(socket)), _in_queue(in_queue)
        {
            _owner = parent;
        }

        ~connection(){}

        void ConnectToClient(uint32_t uid = 0)
        {
            if (_owner == owner::server)
            {
                if (_socket.is_open())
                {
                    _id = uid;
                    ReadHeader();
                }
            }
        }

        void ConnectToServer(const boost::asio::ip::tcp::resolver::results_type& endpoints)
        {
            if (_owner == owner::client)
            {
                boost::asio::async_connect(_socket, endpoints, [this](boost::system::error_code ec, boost::asio::ip::tcp::endpoint endpoint)
                {
                    if (!ec)
                    {
                        ReadHeader();
                    }
                });
            }
        }

        void Disconnect()
        {
            if (IsConnected())
                boost::asio::post(_context, [this]() { _socket.close(); });
        }
        
        bool IsConnected() const
        {
            return _socket.is_open();
        }

        void Send(const message& msg)
        {
            boost::asio::post(_context, [this, msg]()
            {
                bool WritingMessage = !_out_queue.empty();
                _out_queue.push_back(msg);
                if (!WritingMessage)
                {
                    WriteHeader();
                }
            });
        }

        uint32_t GetId() const
        {
            return _id;
        }

        void ReadHeader()
        {
            boost::asio::async_read(_socket, boost::asio::buffer(&_tmp_msg.header, sizeof(mes_header)),
                                    [this](boost::system::error_code ec, size_t length)
                                    {
                                        if (!ec)
                                        {
                                            if (_tmp_msg.header.size > 0)
                                            {
                                                _tmp_msg.body.resize(_tmp_msg.header.size);
                                                ReadBody();
                                            } else {
                                                AddToIncomingMsgQueue();
                                            }
                                        } else {
                                            std::cout << "[" << _id << "] Read header failed: " << ec.message() << "\n";
                                            _socket.close();
                                        }
                                    });
        }

        void ReadBody()
        {
            boost::asio::async_read(_socket, boost::asio::buffer(_tmp_msg.body.data(), _tmp_msg.body.size()),
                                    [this](boost::system::error_code ec, size_t length)
                                    {
                                        if (!ec)
                                        {
                                            std::cout << "[" << _id << "] " << _tmp_msg.body << "\n";
                                            AddToIncomingMsgQueue();
                                        } else {
                                            std::cout << "[" << _id << "] Read body failed: " << ec.message() << "\n";
                                            _socket.close();
                                        }
                                    });
        }

        void WriteHeader()
        {
            boost::asio::async_write(_socket, boost::asio::buffer(&_out_queue.front().header, sizeof(mes_header)),
                                     [this](boost::system::error_code ec, size_t length)
                                     {
                                         if (!ec)
                                         {
                                             if (!_out_queue.front().body.empty())
                                             {
                                                 WriteBody();
                                             } else {
                                                 _out_queue.pop_front();
                                                 if (!_out_queue.empty())
                                                 {
                                                     WriteHeader();
                                                 }
                                             }
                                         } else {
                                             std::cout << "[" << _id << "] Write header failed: " << ec.message() << "\n";
                                             _socket.close();
                                         }
                                     });
        }

        void WriteBody()
        {
            boost::asio::async_write(_socket, boost::asio::buffer(_out_queue.front().body.data(), _out_queue.front().size()),
                                     [this](boost::system::error_code ec, size_t length)
                                     {
                                        if (!ec)
                                        {
                                            _out_queue.pop_front();

                                            if (!_out_queue.empty())
                                            {
                                                WriteHeader();
                                            }
                                        } else {
                                            std::cout << "[" << _id << "] Write body failed: " << ec.message() << "\n";
                                            _socket.close();
                                        }
                                     });
        }

        void AddToIncomingMsgQueue()
        {
            if (_owner == owner::server)
                _in_queue.push_back({ this->shared_from_this(), _tmp_msg });
            else
                _in_queue.push_back({nullptr, _tmp_msg });
            ReadHeader();
        }

    private:
        boost::asio::ip::tcp::socket _socket;
        boost::asio::io_context& _context;
        tsqueue<message> _out_queue;
        tsqueue<owned_message>& _in_queue;
        owner _owner = owner::server;
        uint32_t _id = 0;
        message _tmp_msg;
    };
}

#endif //CLIENT_APP_NETCONNECTION_HPP