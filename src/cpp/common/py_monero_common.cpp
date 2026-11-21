/**
 * Copyright (c) Libmonero
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Parts of this file are originally copyright (c) 2025-2026 woodser
 *
 * Parts of this file are originally copyright (c) 2014-2019, The Monero Project
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * All rights reserved.
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be
 *    used to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Parts of this file are originally copyright (c) 2012-2013 The Cryptonote developers
 */
#include "py_monero_common.h"

// --------------------------- GEN UTILS ---------------------------

std::string PyGenUtils::serialize(const py::dict& d) {
  py::object fn = py::module_::import("json").attr("dumps");
  return fn(d).cast<std::string>();
}

py::dict PyGenUtils::deserialize(const std::string& s) {
  py::object fn = py::module_::import("json").attr("loads");
  return fn(s).cast<py::dict>();
}

py::object PyGenUtils::parse_rpc_response(const std::string& body, bool json_rpc) {
  // parse the raw body with python's own json parsing, which keeps the empty containers and the
  // exact numbers that a property tree walk cannot represent
  py::object response;
  try {
    response = py::module_::import("json").attr("loads")(py::bytes(body));
  } catch (const py::error_already_set& e) {
    // a body that is not json is not a decode error of python, it is a bad answer of the server
    throw monero_error(std::string("Failed to parse the RPC response as json: ") + e.what());
  }
  // a non-object answer carries no json rpc result
  if (!py::isinstance<py::dict>(response)) return json_rpc ? py::none() : response;
  py::dict answer = response;
  // return the result of a json rpc response, which raise_rpc_error checked for an error already
  if (json_rpc) return answer.contains("result") ? py::object(answer["result"]) : py::none();
  // a response that carries the jsonrpc key was already answered as a json rpc one
  return answer.contains("jsonrpc") ? py::none() : response;
}

// --------------------------- MONERO REQUEST PARAMS ---------------------------

rapidjson::Value PyMoneroRequestParams::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {
  rapidjson::Value root(rapidjson::kObjectType);
  if (m_py_params == boost::none) return root;

  // convert python dict params to rapidjson::Value
  std::string json = PyGenUtils::serialize(m_py_params.get());
  rapidjson::Document doc;
  doc.Parse(json.c_str());
  root.CopyFrom(doc, allocator);

  return root;
}
