#include "tcp/inet_addr.h"
#include "gtest/gtest.h"

TEST(InetAddrTest, Loopback) {
    InetAddr addr(12345);
    EXPECT_EQ(addr.Ip(), "0.0.0.0");
    EXPECT_EQ(addr.HostOrderPort(), 12345);
}

TEST(InetAddrTest, SpecificIP) {
    InetAddr addr("127.0.0.1", 8080);
    EXPECT_EQ(addr.Ip(), "127.0.0.1");
    EXPECT_EQ(addr.IpPort(), "127.0.0.1:8080");
    EXPECT_EQ(addr.HostOrderPort(), 8080);
}

TEST(InetAddrTest, NetworkOrder) {
    InetAddr addr("192.168.1.1", 9999);
    EXPECT_EQ(addr.NetworkOrderIp(), inet_addr("192.168.1.1"));
    EXPECT_EQ(addr.NetworkOrderPort(), htons(9999));
}

TEST(InetAddrTest, CopyConstructor) {
    InetAddr addr1("10.0.0.1", 443);
    InetAddr addr2(addr1);
    EXPECT_EQ(addr2.IpPort(), "10.0.0.1:443");
}
