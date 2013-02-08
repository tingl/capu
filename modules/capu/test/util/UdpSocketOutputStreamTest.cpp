/*
 * Copyright (C) 2012 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "UdpSocketOutputStreamTest.h"

#include <capu/util/IInputStream.h>
#include <capu/os/Math.h>
#include <capu/util/Runnable.h>
#include <capu/os/Thread.h>

namespace capu
{
    struct TestUdpSocketReceiver: public IInputStream
    {
        TestUdpSocketReceiver(UdpSocket& socket)
            : mSocket(socket)
        {
        }

        UdpSocket& mSocket;

        void receiveFromSocket(char_t* data, const uint32_t size)
        {
            int32_t length = 0;
            uint32_t receivedBytes = 0;
            char_t* buffer = static_cast<char_t*>(data);
            while (receivedBytes < size)
            {
                SocketAddrInfo senderInfo;
                status_t result = mSocket.receive(&buffer[receivedBytes], size - receivedBytes, length, &senderInfo);
                if (result != CAPU_OK)
                {
                    break;
                }

                if (0 == length)
                {
                    break;
                }

                receivedBytes += length;
            }
        }

        IInputStream& operator>>(float_t& value)
        {
            return operator>>(reinterpret_cast<int32_t&>(value));
        }

        IInputStream& operator>>(int32_t& value)
        {
            int32_t tmp;
            receiveFromSocket(reinterpret_cast<char_t*>(&tmp), sizeof(int32_t));
            value = ntohl(tmp);
            return *this;
        }

        IInputStream& operator>>(uint32_t& value)
        {
            int32_t tmp;
            operator>>(tmp);
            value = static_cast<uint32_t>(tmp);
            return *this;
        }

        IInputStream& operator>>(String& value)
        {
            char_t buffer[1024];
            receiveFromSocket(buffer, 15);
            int32_t strLen = ntohl(*reinterpret_cast<int32_t*>(buffer));
            char_t* strBuf = &buffer[sizeof(int32_t)];
            strBuf[strLen] = 0;
            value = strBuf;
            return *this;
        }

        IInputStream& operator>>(bool_t& value)
        {
            receiveFromSocket(reinterpret_cast<char_t*>(&value), sizeof(bool_t));
            return *this;
        }

        IInputStream& operator>>(uint16_t& value)
        {
            receiveFromSocket(reinterpret_cast<char_t*>(&value), sizeof(uint16_t));
            value = ntohs(value);
            return *this;
        }

        IInputStream& operator>>(Guid& value)
        {
            generic_uuid_t tmpValue;
            read(reinterpret_cast<char_t*>(&tmpValue), sizeof(tmpValue));
            value = tmpValue;
            return *this;
        }

        IInputStream& read(char_t* data, const uint32_t size)
        {
            receiveFromSocket(data, size);
            return *this;
        }
    };

    template<typename T>
    class UdpSocketOutputStreamTestRunnable: public Runnable
    {
    public:
        UdpSocketOutputStreamTestRunnable()
        {
            serverSocket.bind(0, 0);
            mPort = serverSocket.getSocketAddrInfo().port;
        }

        T result;
        void run()
        {
            TestUdpSocketReceiver receiver(serverSocket);
            receiver >> result;

            serverSocket.close();
        }

        UdpSocket serverSocket;
        uint16_t mPort;
    };

    UdpSocketOutputStreamTest::UdpSocketOutputStreamTest()
    {
    }

    UdpSocketOutputStreamTest::~UdpSocketOutputStreamTest()
    {
    }

    void UdpSocketOutputStreamTest::SetUp()
    {
    }

    void UdpSocketOutputStreamTest::TearDown()
    {
    }

    template<typename T>
    struct UdpSocketOutputStreamTestExecutor
    {
        static T Execute(const T& value)
        {
            UdpSocketOutputStreamTestRunnable<T> runnable;
            Thread thread;
            thread.start(runnable);

            Thread::Sleep(1);  // give receiving thread a chance to get cpu time

            UdpSocket socket;
            UdpSocketOutputStream<1450> outputStream(socket, "127.0.0.1", runnable.mPort);

            outputStream << value;
            outputStream.flush();

            thread.join();

            return runnable.result;
        }
    };

    TEST_F(UdpSocketOutputStreamTest, SendInt32Data)
    {
        EXPECT_EQ(5, UdpSocketOutputStreamTestExecutor<int32_t>::Execute(5));
    }

    TEST_F(UdpSocketOutputStreamTest, SendUInt32Data)
    {
        EXPECT_EQ(5u, UdpSocketOutputStreamTestExecutor<uint32_t>::Execute(5u));
    }

    TEST_F(UdpSocketOutputStreamTest, SendFloatData)
    {
        EXPECT_EQ(Math::PI_f, UdpSocketOutputStreamTestExecutor<float_t>::Execute(Math::PI_f));
    }

    TEST_F(UdpSocketOutputStreamTest, SendStringData)
    {
        EXPECT_STREQ("Hello World", UdpSocketOutputStreamTestExecutor<String>::Execute("Hello World"));
    }

    TEST_F(UdpSocketOutputStreamTest, SendUInt16Data)
    {
        EXPECT_EQ((uint16_t)(4), UdpSocketOutputStreamTestExecutor<uint16_t>::Execute(4));
    }

    TEST_F(UdpSocketOutputStreamTest, SendGuidData)
    {
        Guid guid;
        EXPECT_EQ(guid, UdpSocketOutputStreamTestExecutor<Guid>::Execute(guid));
    }

    TEST_F(UdpSocketOutputStreamTest, SendBoolData)
    {
        EXPECT_EQ(true, UdpSocketOutputStreamTestExecutor<bool_t>::Execute(true));
    }

    TEST_F(UdpSocketOutputStreamTest, TestStreamInitialStateIsOk)
    {
        UdpSocket socket;
        UdpSocketOutputStream<1450> stream(socket, "127.0.0.1", 55555);
        EXPECT_EQ(CAPU_OK, stream.getState());
    }

    TEST_F(UdpSocketOutputStreamTest, TestGetAddrInfo)
    {
        UdpSocket socket;
        UdpSocketOutputStream<1450> stream(socket, "127.0.0.1", 55555);
        const SocketAddrInfo& addrInfo = stream.getAddrInfo();

        EXPECT_EQ(55555, addrInfo.port);
        EXPECT_STREQ("127.0.0.1", addrInfo.addr);
    }
}