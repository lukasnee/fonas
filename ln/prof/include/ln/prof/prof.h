// Copyright (c) 2026 Lukas Neverauskis <lukas.neverauskis@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#ifndef LN_PROF

#define LN_PROF_ENTER(id)
#define LN_PROF_EXIT(id)
#define LN_PROF_SCOPE()

#else

#ifdef __cplusplus
extern "C"
{
#endif

    extern "C" __attribute__((optimize("O3"), hot)) void
    __cyg_profile_func_enter(void *this_fn, void *call_site);
    extern "C" __attribute__((optimize("O3"), hot)) void
    __cyg_profile_func_exit(void *this_fn, void *call_site);

#define LN_PROF_ENTER(id) __cyg_profile_func_enter((void *)id, NULL)
#define LN_PROF_EXIT(id) __cyg_profile_func_exit((void *)id, NULL)

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

namespace ln::prof {

class Scope {
public:
    Scope(int id) : m_id{id} { LN_PROF_ENTER(m_id); }
    ~Scope() { LN_PROF_EXIT(m_id); }

private:
    int m_id;
};

} // namespace ln::prof
#define LN_PROF_CONCAT_IMPL(x, y) x##y
#define LN_PROF_CONCAT(x, y) LN_PROF_CONCAT_IMPL(x, y)
#define LN_PROF_SCOPE()                                                        \
    ln::prof::Scope LN_PROF_CONCAT(__ln_prof_scope_, __LINE__) { __LINE__ }

#endif // __cplusplus
#endif