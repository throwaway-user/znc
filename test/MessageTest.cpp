/*
 * Copyright (C) 2004-2015 ZNC, see the NOTICE file for details.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <znc/ZNCString.h>
#include <znc/Message.h>

TEST(MessageTest, SetParam) {
	CMessage msg;

	msg.SetParam(1, "bar");
	msg.SetParam(0, "foo");

	VCString params = {"foo", "bar"};
	EXPECT_EQ(params, msg.GetParams());

	msg.SetParam(3, "baz");

	params = {"foo", "bar", "", "baz"};
	EXPECT_EQ(params, msg.GetParams());
}

TEST(MessageTest, ToString) {
	EXPECT_EQ("CMD", CMessage("CMD").ToString());
	EXPECT_EQ("CMD p1", CMessage("CMD p1").ToString());
	EXPECT_EQ("CMD p1 p2", CMessage("CMD p1 p2").ToString());
	EXPECT_EQ("CMD :p p p", CMessage("CMD :p p p").ToString());
	EXPECT_EQ(":irc.znc.in", CMessage(":irc.znc.in").ToString());
	EXPECT_EQ(":irc.znc.in CMD", CMessage(":irc.znc.in CMD").ToString());
	EXPECT_EQ(":irc.znc.in CMD p1", CMessage(":irc.znc.in CMD p1").ToString());
	EXPECT_EQ(":irc.znc.in CMD p1 p2", CMessage(":irc.znc.in CMD p1 p2").ToString());
	EXPECT_EQ(":irc.znc.in CMD :p p p", CMessage(":irc.znc.in CMD :p p p").ToString());
	EXPECT_EQ(":irc.znc.in CMD :", CMessage(":irc.znc.in CMD :").ToString());

	EXPECT_EQ("CMD", CMessage(CNick(), "CMD").ToString());
	EXPECT_EQ("CMD p1", CMessage(CNick(), "CMD", {"p1"}).ToString());
	EXPECT_EQ("CMD p1 p2", CMessage(CNick(), "CMD", {"p1", "p2"}).ToString());
	EXPECT_EQ("CMD :p p p", CMessage(CNick(), "CMD", {"p p p"}).ToString());
	EXPECT_EQ(":irc.znc.in", CMessage(CNick(":irc.znc.in"), "").ToString());
	EXPECT_EQ(":irc.znc.in CMD", CMessage(CNick(":irc.znc.in"), "CMD").ToString());
	EXPECT_EQ(":irc.znc.in CMD p1", CMessage(CNick(":irc.znc.in"), "CMD", {"p1"}).ToString());
	EXPECT_EQ(":irc.znc.in CMD p1 p2", CMessage(CNick(":irc.znc.in"), "CMD", {"p1", "p2"}).ToString());
	EXPECT_EQ(":irc.znc.in CMD :p p p", CMessage(CNick(":irc.znc.in"), "CMD", {"p p p"}).ToString());
	EXPECT_EQ(":irc.znc.in CMD :", CMessage(CNick(":irc.znc.in"), "CMD", {""}).ToString());
}

TEST(MessageTest, FormatFlags) {
	const CString line = "@foo=bar :irc.example.com COMMAND param";

	CMessage msg(line);
	EXPECT_EQ(line, msg.ToString());
	EXPECT_EQ(":irc.example.com COMMAND param", msg.ToString(CMessage::ExcludeTags));
	EXPECT_EQ("@foo=bar COMMAND param", msg.ToString(CMessage::ExcludePrefix));
	EXPECT_EQ("COMMAND param", msg.ToString(CMessage::ExcludePrefix|CMessage::ExcludeTags));
}

TEST(MessageTest, ChanAction) {
	CMessage msg(":sender PRIVMSG #chan :\001ACTION ACTS\001");
	CActionMessage& chan = static_cast<CActionMessage&>(msg);
	EXPECT_EQ("sender", chan.GetNick().GetNick());
	EXPECT_EQ("PRIVMSG", chan.GetCommand());
	EXPECT_EQ("#chan", chan.GetTarget());
	EXPECT_EQ("ACTS", chan.GetText());

	chan.SetTarget("#znc");
	EXPECT_EQ("#znc", chan.GetTarget());
	chan.SetText("foo bar");
	EXPECT_EQ("foo bar", chan.GetText());
	EXPECT_EQ(":sender PRIVMSG #znc :\001ACTION foo bar\001", chan.ToString());
}

TEST(MessageTest, ChanCTCP) {
	CMessage msg(":sender PRIVMSG #chan :\001text\001");
	CCTCPMessage& chan = static_cast<CCTCPMessage&>(msg);
	EXPECT_EQ("sender", chan.GetNick().GetNick());
	EXPECT_EQ("PRIVMSG", chan.GetCommand());
	EXPECT_EQ("#chan", chan.GetTarget());
	EXPECT_EQ("text", chan.GetText());

	chan.SetTarget("#znc");
	EXPECT_EQ("#znc", chan.GetTarget());
	chan.SetText("foo bar");
	EXPECT_EQ("foo bar", chan.GetText());
	EXPECT_EQ(":sender PRIVMSG #znc :\001foo bar\001", chan.ToString());
}

TEST(MessageTest, ChanMsg) {
	CMessage msg(":sender PRIVMSG #chan :text");
	CTextMessage& chan = static_cast<CTextMessage&>(msg);
	EXPECT_EQ("sender", chan.GetNick().GetNick());
	EXPECT_EQ("PRIVMSG", chan.GetCommand());
	EXPECT_EQ("#chan", chan.GetTarget());
	EXPECT_EQ("text", chan.GetText());

	chan.SetTarget("#znc");
	EXPECT_EQ("#znc", chan.GetTarget());
	chan.SetText("foo bar");
	EXPECT_EQ("foo bar", chan.GetText());
	EXPECT_EQ(":sender PRIVMSG #znc :foo bar", chan.ToString());
}

TEST(MessageTest, Kick) {
	CMessage msg(":nick KICK #chan person :reason");
	CKickMessage& kick = static_cast<CKickMessage&>(msg);
	EXPECT_EQ("nick", kick.GetNick().GetNick());
	EXPECT_EQ("KICK", kick.GetCommand());
	EXPECT_EQ("#chan", kick.GetTarget());
	EXPECT_EQ("person", kick.GetKickedNick());
	EXPECT_EQ("reason", kick.GetReason());

	kick.SetTarget("#znc");
	EXPECT_EQ("#znc", kick.GetTarget());
	kick.SetKickedNick("noone");
	EXPECT_EQ("noone", kick.GetKickedNick());
	kick.SetReason("test");
	EXPECT_EQ("test", kick.GetReason());
	EXPECT_EQ(":nick KICK #znc noone test", kick.ToString());
}

TEST(MessageTest, Join) {
	CMessage msg(":nick JOIN #chan");
	CJoinMessage& join = static_cast<CJoinMessage&>(msg);
	EXPECT_EQ("nick", join.GetNick().GetNick());
	EXPECT_EQ("JOIN", join.GetCommand());
	EXPECT_EQ("#chan", join.GetTarget());

	join.SetTarget("#znc");
	EXPECT_EQ("#znc", join.GetTarget());
	EXPECT_EQ(":nick JOIN #znc", join.ToString());
}

TEST(MessageTest, Nick) {
	CMessage msg(":nick NICK person");
	CNickMessage& nick = static_cast<CNickMessage&>(msg);
	EXPECT_EQ("nick", nick.GetNick().GetNick());
	EXPECT_EQ("NICK", nick.GetCommand());
	EXPECT_EQ("nick", nick.GetOldNick());
	EXPECT_EQ("person", nick.GetNewNick());

	nick.SetNewNick("test");
	EXPECT_EQ("test", nick.GetNewNick());
	EXPECT_EQ(":nick NICK test", nick.ToString());
}

TEST(MessageTest, Part) {
	CMessage msg(":nick PART #chan :reason");
	CPartMessage& part = static_cast<CPartMessage&>(msg);
	EXPECT_EQ("nick", part.GetNick().GetNick());
	EXPECT_EQ("PART", part.GetCommand());
	EXPECT_EQ("#chan", part.GetTarget());
	EXPECT_EQ("reason", part.GetReason());

	part.SetTarget("#znc");
	EXPECT_EQ("#znc", part.GetTarget());
	part.SetReason("test");
	EXPECT_EQ("test", part.GetReason());
	EXPECT_EQ(":nick PART #znc test", part.ToString());
}

TEST(MessageTest, PrivAction) {
	CMessage msg(":sender PRIVMSG receiver :\001ACTION ACTS\001");
	CActionMessage& priv = static_cast<CActionMessage&>(msg);
	EXPECT_EQ("sender", priv.GetNick().GetNick());
	EXPECT_EQ("PRIVMSG", priv.GetCommand());
	EXPECT_EQ("receiver", priv.GetTarget());
	EXPECT_EQ("ACTS", priv.GetText());

	priv.SetTarget("noone");
	EXPECT_EQ("noone", priv.GetTarget());
	priv.SetText("foo bar");
	EXPECT_EQ("foo bar", priv.GetText());
	EXPECT_EQ(":sender PRIVMSG noone :\001ACTION foo bar\001", priv.ToString());
}

TEST(MessageTest, PrivCTCP) {
	CMessage msg(":sender PRIVMSG receiver :\001text\001");
	CCTCPMessage& priv = static_cast<CCTCPMessage&>(msg);
	EXPECT_EQ("sender", priv.GetNick().GetNick());
	EXPECT_EQ("PRIVMSG", priv.GetCommand());
	EXPECT_EQ("receiver", priv.GetTarget());
	EXPECT_EQ("text", priv.GetText());

	priv.SetTarget("noone");
	EXPECT_EQ("noone", priv.GetTarget());
	priv.SetText("foo bar");
	EXPECT_EQ("foo bar", priv.GetText());
	EXPECT_EQ(":sender PRIVMSG noone :\001foo bar\001", priv.ToString());
}

TEST(MessageTest, PrivMsg) {
	CMessage msg(":sender PRIVMSG receiver :foo bar");
	CTextMessage& priv = static_cast<CTextMessage&>(msg);
	EXPECT_EQ("sender", priv.GetNick().GetNick());
	EXPECT_EQ("PRIVMSG", priv.GetCommand());
	EXPECT_EQ("receiver", priv.GetTarget());
	EXPECT_EQ("foo bar", priv.GetText());

	priv.SetTarget("noone");
	EXPECT_EQ("noone", priv.GetTarget());
	priv.SetText(":)");
	EXPECT_EQ(":)", priv.GetText());
	EXPECT_EQ(":sender PRIVMSG noone ::)", priv.ToString());
}

TEST(MessageTest, Quit) {
	CMessage msg(":nick QUIT :reason");
	CQuitMessage& quit = static_cast<CQuitMessage&>(msg);
	EXPECT_EQ("nick", quit.GetNick().GetNick());
	EXPECT_EQ("QUIT", quit.GetCommand());
	EXPECT_EQ("reason", quit.GetReason());

	quit.SetReason("test");
	EXPECT_EQ("test", quit.GetReason());
	EXPECT_EQ(":nick QUIT test", quit.ToString());
}

TEST(MessageTest, Topic) {
	CMessage msg(":nick TOPIC #chan :topic");
	CTopicMessage& topic = static_cast<CTopicMessage&>(msg);
	EXPECT_EQ("nick", topic.GetNick().GetNick());
	EXPECT_EQ("TOPIC", topic.GetCommand());
	EXPECT_EQ("#chan", topic.GetTarget());
	EXPECT_EQ("topic", topic.GetTopic());

	topic.SetTarget("#znc");
	EXPECT_EQ("#znc", topic.GetTarget());
	topic.SetTopic("test");
	EXPECT_EQ("test", topic.GetTopic());
	EXPECT_EQ(":nick TOPIC #znc test", topic.ToString());
}

TEST(MessageTest, Parse) {
	CMessage msg;

	// #1037
	msg.Parse(":irc.znc.in PRIVMSG ::)");
	EXPECT_EQ(":)", msg.GetParam(0));
}

// The test data for MessageTest.Parse originates from https://github.com/SaberUK/ircparser
//
// IRCParser - Internet Relay Chat Message Parser
//
//   Copyright (C) 2015 Peter "SaberUK" Powell <petpow@saberuk.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without
// fee is hereby granted, provided that the above copyright notice and this permission notice appear
// in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
// SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
// AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
// NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
// OF THIS SOFTWARE.

// when checking a valid message with tags and a source
TEST(MessageTest, ParseWithTags) {
	const CString line = "@tag1=value1;tag2;vendor1/tag3=value2;vendor2/tag4 :irc.example.com COMMAND param1 param2 :param3 param3";

	MCString tags;
	tags["tag1"] = "value1";
	tags["tag2"] = "";
	tags["vendor1/tag3"] = "value2";
	tags["vendor2/tag4"] = "";

	VCString params = {"param1", "param2", "param3 param3"};

	CMessage msg(line);
	EXPECT_EQ(line, msg.ToString());
	EXPECT_EQ(tags, msg.GetTags());
	EXPECT_EQ("irc.example.com", msg.GetNick().GetNick());
	EXPECT_EQ("COMMAND", msg.GetCommand());
	EXPECT_EQ(params, msg.GetParams());
}

// when checking a valid message with a source but no tags
TEST(MessageTest, ParseWithoutTags) {
	const CString line = ":irc.example.com COMMAND param1 param2 :param3 param3";

	VCString params = {"param1", "param2", "param3 param3"};

	CMessage msg(line);
	EXPECT_EQ(line, msg.ToString());
	EXPECT_EQ(MCString(), msg.GetTags());
	EXPECT_EQ("irc.example.com", msg.GetNick().GetNick());
	EXPECT_EQ("COMMAND", msg.GetCommand());
	EXPECT_EQ(params, msg.GetParams());
}

// when checking a valid message with tags but no source
TEST(MessageTest, ParseWithoutSource) {
	const CString line = "@tag1=value1;tag2;vendor1/tag3=value2;vendor2/tag4 COMMAND param1 param2 :param3 param3";

	MCString tags;
	tags["tag1"] = "value1";
	tags["tag2"] = "";
	tags["vendor1/tag3"] = "value2";
	tags["vendor2/tag4"] = "";

	VCString params = {"param1", "param2", "param3 param3"};

	CMessage msg(line);
	EXPECT_EQ(line, msg.ToString());
	EXPECT_EQ(tags, msg.GetTags());
	EXPECT_EQ("", msg.GetNick().GetNick());
	EXPECT_EQ("COMMAND", msg.GetCommand());
	EXPECT_EQ(params, msg.GetParams());
}

// when checking a valid message with no tags, source or parameters
TEST(MessageTest, ParseWithoutSourceAndTags) {
	const CString line = "COMMAND";

	CMessage msg(line);
	EXPECT_EQ(line, msg.ToString());
	EXPECT_EQ(MCString(), msg.GetTags());
	EXPECT_EQ("", msg.GetNick().GetNick());
	EXPECT_EQ("COMMAND", msg.GetCommand());
	EXPECT_EQ(VCString(), msg.GetParams());
}
