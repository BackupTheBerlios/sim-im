/*
 * XML Parser/Generator
 * Simple XML parser/generator sufficient to
 * send+receive the XML in ICQ SMS messages
 * Copyright (C) 2001 Barnaby Gray <barnaby@beedesign.co.uk>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef XML_H
#define XML_H

#include <list>
#include <string>
#include <ctype.h>

class XmlNode {
private:
    static std::string parseTag(std::string::iterator& curr, std::string::iterator end);
    static void skipWS(std::string::iterator& curr, std::string::iterator end);

protected:
    std::string tag;

    XmlNode(const std::string& t);

public:
    virtual ~XmlNode();

    virtual bool isBranch() = 0;
    bool isLeaf();

    std::string getTag();

    static XmlNode *parse(std::string::iterator& start, std::string::iterator end);

    static std::string quote(const std::string& s);
    static std::string unquote(const std::string& s);
    static std::string replace_all(const std::string& s, const std::string& r1, const std::string& r2);

    virtual std::string toString(int n) = 0;
};

class XmlLeaf;

class XmlBranch : public XmlNode {
private:
    std::list<XmlNode*> children;

public:
    XmlBranch(const std::string& t);
    ~XmlBranch();

    bool isBranch();
    bool exists(const std::string& tag);
    XmlNode *getNode(const std::string& tag);
    XmlBranch *getBranch(const std::string& tag);
    XmlLeaf *getLeaf(const std::string& tag);

    void pushnode(XmlNode *c);

    std::string toString(int n);

};

class XmlLeaf : public XmlNode {
private:
    std::string value;
public:
    XmlLeaf(const std::string& t, const std::string& v);
    ~XmlLeaf();

    bool isBranch();
    std::string getValue();

    std::string toString(int n);

};

#endif

