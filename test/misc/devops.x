﻿#
# Copyright (C) 2024 The XLang Foundation
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# <END>

#define DevOps_Code  R"(
from xlang_http import http
print("DevOps Server Started")
mime = "text/html"
notify_list =[]

handlder = (evt){
    extern notify_list;
	notifyInfo ="";
	action = evt.getattr("action");
	if action == "end":
		notifyInfo = "end"
	elif action == "notify":
		param = evt.getattr("param")
		notifyInfo = "$notify$" + param
	print("devops.dbg fired:",notifyInfo);
	notify_list+=notifyInfo;
}
on('devops.dbg',handlder)

srv = http.Server()
srv.get("/devops/run",(req,res){
  path = req.get_path();
  params = req.get_params();
  code = params['code'];
  print("code:{\n",code,"\n}");
  retData = runcode("devops_run.x",code);
  retStr = tostring(retData,format=True);
  print("send back:",retStr);
  res.set_content(retStr, mime);
})

srv.get("/devops/getnotify",(req,res){
  extern notify_list;
  path = req.get_path();
  print(path);
  while notify_list.size() == 0:
	sleep(100)
  retStr = notify_list[0];
  notify_list.remove(0);
  res.set_content(retStr, mime);
  print("Send back:",retStr);
})

srv.get("/stop",(req,res){
  print("stop....")
  srv.stop();
})

srv.listen("::", 3141)
print("DevOps Server Stopped")
#)"