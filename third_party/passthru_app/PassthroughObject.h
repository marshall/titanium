// Copyright 2007 Igor Tandetnik
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GEARS_THIRD_PARTY_PASSTHRU_APP_PASSTHROUGHOBJECT_H__
#define GEARS_THIRD_PARTY_PASSTHRU_APP_PASSTHROUGHOBJECT_H__

// {C38D254C-4C40-4192-A746-AC6FE519831E}
extern "C" const __declspec(selectany) IID IID_IPassthroughObject =
  {0xc38d254c, 0x4c40, 0x4192,
    {0xa7, 0x46, 0xac, 0x6f, 0xe5, 0x19, 0x83, 0x1e}};

struct
__declspec(uuid("{C38D254C-4C40-4192-A746-AC6FE519831E}"))
__declspec(novtable)
IPassthroughObject : public IUnknown
{
  STDMETHOD(SetTargetUnknown)(IUnknown* punkTarget) = 0;
};

#if _ATL_VER < 0x700
  #define InlineIsEqualGUID ::ATL::InlineIsEqualGUID
#else
  #define InlineIsEqualGUID ::InlineIsEqualGUID
#endif

#endif  // GEARS_THIRD_PARTY_PASSTHRU_APP_PASSTHROUGHOBJECT_H__
