/*
 * Copyright (c) 2015 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <vnet/policer/policer.h>

u8 * format_policer_instance (u8 * s, va_list * va)
{
  policer_read_response_type_st * i 
    = va_arg (*va, policer_read_response_type_st *);

  s = format (s, "policer at %llx: %s rate, %s color-aware\n", 
              i, i->single_rate ? "single" : "dual",
              i->color_aware ? "is" : "not");
  s = format (s, "cir %u tok/period, pir %u tok/period, scale %u\n",
              i->cir_tokens_per_period, i->pir_tokens_per_period, 
              i->scale);
  s = format (s, "cur lim %u, cur bkt %u, ext lim %u, ext bkt %u\n",
              i->current_limit,
              i->current_bucket,
              i->extended_limit,
              i->extended_bucket);
  s = format (s, "last update %llu\n", i->last_update_time);
  return s;
}              

static u8 * format_policer_round_type (u8 * s, va_list * va)
{
  sse2_qos_pol_cfg_params_st * c 
    = va_arg (*va, sse2_qos_pol_cfg_params_st *);

  if (c->rnd_type == SSE2_QOS_ROUND_TO_CLOSEST)
    s = format(s, "closest");
  else if (c->rnd_type == SSE2_QOS_ROUND_TO_UP)
    s = format (s, "up");
  else if (c->rnd_type == SSE2_QOS_ROUND_TO_DOWN)
    s = format (s, "down");
  else
    s = format (s, "ILLEGAL");
  return s;
}


static u8 * format_policer_rate_type (u8 * s, va_list * va)
{
  sse2_qos_pol_cfg_params_st * c 
    = va_arg (*va, sse2_qos_pol_cfg_params_st *);

  if (c->rate_type == SSE2_QOS_RATE_KBPS)
    s = format (s, "kbps");
  else if (c->rate_type == SSE2_QOS_RATE_PPS)
    s = format(s, "pps");
  else
    s = format (s, "ILLEGAL");
  return s;
}

static u8 * format_policer_type (u8 * s, va_list * va)
{
  sse2_qos_pol_cfg_params_st * c 
    = va_arg (*va, sse2_qos_pol_cfg_params_st *);
  
  if (c->rfc == SSE2_QOS_POLICER_TYPE_1R2C)
    s = format (s, "1r2c");
      
  else if (c->rfc == SSE2_QOS_POLICER_TYPE_1R3C_RFC_2697)
    s = format (s, "1r3c");

  else if (c->rfc == SSE2_QOS_POLICER_TYPE_2R3C_RFC_2698)
    s = format (s, "2r3c-2698");

  else if (c->rfc == SSE2_QOS_POLICER_TYPE_2R3C_RFC_4115)
    s = format (s, "2r3c-4115");

  else if (c->rfc == SSE2_QOS_POLICER_TYPE_2R3C_RFC_MEF5CF1)
    s = format (s, "2r3c-mef5cf1");
  else
    s = format (s, "ILLEGAL");
  return s;
}

u8 * format_policer_config (u8 * s, va_list * va)
{
  sse2_qos_pol_cfg_params_st * c 
    = va_arg (*va, sse2_qos_pol_cfg_params_st *);

  s = format (s, "type %U cir %u eir %u cb %u eb %u\n",
              format_policer_type, c, 
              c->rb.kbps.cir_kbps,
              c->rb.kbps.eir_kbps,
              c->rb.kbps.cb_bytes,
              c->rb.kbps.eb_bytes);
  s = format (s, "rate type %U, round type %U\n",
              format_policer_rate_type, c,
              format_policer_round_type, c);
  return s;
}

static uword
unformat_policer_type (unformat_input_t * input, va_list * va)
{
  sse2_qos_pol_cfg_params_st * c 
    = va_arg (*va, sse2_qos_pol_cfg_params_st *);

  if (!unformat (input, "type"))
    return 0;

  if (unformat (input, "1r2c"))
    c->rfc = SSE2_QOS_POLICER_TYPE_1R2C;
  else if (unformat (input, "1r3c"))
    c->rfc = SSE2_QOS_POLICER_TYPE_1R3C_RFC_2697;
  else if (unformat (input, "2r3c-2698"))
    c->rfc = SSE2_QOS_POLICER_TYPE_2R3C_RFC_2698;
  else if (unformat (input, "2r3c-4115"))
    c->rfc = SSE2_QOS_POLICER_TYPE_2R3C_RFC_4115;
  else if (unformat (input, "2r3c-mef5cf1"))
    c->rfc = SSE2_QOS_POLICER_TYPE_2R3C_RFC_MEF5CF1;
  else
    return 0;
  return 1;
}

static uword
unformat_policer_round_type (unformat_input_t * input, va_list * va)
{
  sse2_qos_pol_cfg_params_st * c 
    = va_arg (*va, sse2_qos_pol_cfg_params_st *);

  if (!unformat(input, "round"))
    return 0;

  if (unformat(input, "closest"))
    c->rnd_type = SSE2_QOS_ROUND_TO_CLOSEST;
  else if (unformat (input, "up"))
    c->rnd_type = SSE2_QOS_ROUND_TO_UP;
  else if (unformat (input, "down"))
    c->rnd_type = SSE2_QOS_ROUND_TO_DOWN;
  else
    return 0;
  return 1;
}

static uword
unformat_policer_rate_type (unformat_input_t * input, va_list * va)
{
  sse2_qos_pol_cfg_params_st * c 
    = va_arg (*va, sse2_qos_pol_cfg_params_st *);

  if (!unformat(input, "rate"))
    return 0;

  if (unformat (input, "kbps"))
    c->rate_type = SSE2_QOS_RATE_KBPS;
  else if (unformat(input, "pps"))
    c->rate_type = SSE2_QOS_RATE_PPS;
  else
    return 0;
  return 1;
}

static uword
unformat_policer_cir (unformat_input_t * input, va_list * va)
{
  sse2_qos_pol_cfg_params_st * c 
    = va_arg (*va, sse2_qos_pol_cfg_params_st *);

  if (unformat (input, "cir %u", &c->rb.kbps.cir_kbps))
    return 1;
  return 0;
}

static uword
unformat_policer_eir (unformat_input_t * input, va_list * va)
{
  sse2_qos_pol_cfg_params_st * c 
    = va_arg (*va, sse2_qos_pol_cfg_params_st *);

  if (unformat (input, "eir %u", &c->rb.kbps.eir_kbps))
    return 1;
  return 0;
}

static uword
unformat_policer_cb (unformat_input_t * input, va_list * va)
{
  sse2_qos_pol_cfg_params_st * c 
    = va_arg (*va, sse2_qos_pol_cfg_params_st *);

  if (unformat (input, "cb %u", &c->rb.kbps.cb_bytes))
    return 1;
  return 0;
}

static uword
unformat_policer_eb (unformat_input_t * input, va_list * va)
{
  sse2_qos_pol_cfg_params_st * c 
    = va_arg (*va, sse2_qos_pol_cfg_params_st *);

  if (unformat (input, "eb %u", &c->rb.kbps.eb_bytes))
    return 1;
  return 0;
}


#define foreach_config_param                    \
_(eb)                                           \
_(cb)                                           \
_(eir)                                          \
_(cir)                                          \
_(rate_type)                                    \
_(round_type)                                   \
_(type)

static clib_error_t *
configure_policer_command_fn (vlib_main_t * vm,
                              unformat_input_t * input,
                              vlib_cli_command_t * cmd)
{
  vnet_policer_main_t *pm = &vnet_policer_main;
  sse2_qos_pol_cfg_params_st c;
  policer_read_response_type_st test_policer;
  unformat_input_t _line_input, * line_input = &_line_input;
  int is_add = 1;
  int rv;
  u8 * name = 0;
  uword * p;

  /* Get a line of input. */
  if (! unformat_user (input, unformat_line_input, line_input))
    return 0;

  memset (&c, 0, sizeof (c));

  while (unformat_check_input (line_input) != UNFORMAT_END_OF_INPUT) 
    {
      if (unformat (line_input, "del"))
        is_add = 0;
      else if (unformat(line_input, "name %s", &name))
        ;

#define _(a) else if (unformat (line_input, "%U", unformat_policer_##a, &c)) ;
      foreach_config_param
#undef _

      else
        return clib_error_return (0, "unknown input `%U'",
                                  format_unformat_error, line_input);
    }

  unformat_free (line_input);

  if (is_add == 0)
    {
      p = hash_get_mem (pm->policer_config_by_name, name);
      if (p == 0)
        {
          vec_free(name);
          return clib_error_return (0, "No such policer configuration");
        }
      hash_unset_mem (pm->policer_config_by_name, name);
      vec_free(name);
      return 0;
    }

  /* Vet the configuration before adding it to the table */
  rv = sse2_pol_logical_2_physical (&c, &test_policer);
  
  if (rv == 0)
    {
      policer_read_response_type_st *pp;
      sse2_qos_pol_cfg_params_st *cp;

      pool_get (pm->configs, cp);
      pool_get (pm->policer_templates, pp);

      ASSERT (cp - pm->configs == pp - pm->policer_templates);

      memcpy (cp, &c, sizeof (*cp));
      memcpy (pp, &test_policer, sizeof (*pp));

      hash_set_mem (pm->policer_config_by_name, name, cp - pm->configs);
    }
  else
    {
      vec_free (name);
      return clib_error_return (0, "Config failed sanity check");
    }
  
  return 0;
}

VLIB_CLI_COMMAND (configure_policer_command, static) = {
    .path = "configure policer",
    .short_help = "configure policer name <name> <params> ",
    .function = configure_policer_command_fn,
};


static clib_error_t *
show_policer_command_fn (vlib_main_t * vm,
                              unformat_input_t * input,
                              vlib_cli_command_t * cmd)
{
  vnet_policer_main_t *pm = &vnet_policer_main;
  hash_pair_t * p;
  u32 pool_index;
  u8 * match_name = 0;
  u8 * name;
  sse2_qos_pol_cfg_params_st *config;
  policer_read_response_type_st *templ;

  (void) unformat (input, "name %s", &match_name);

  hash_foreach_pair (p, pm->policer_config_by_name,
  ({
    name = (u8 *) p->key;
    if (match_name == 0 || !strcmp((char *) name, (char *) match_name))
      {
        pool_index = p->value[0];
        config = pool_elt_at_index (pm->configs, pool_index);
        templ = pool_elt_at_index (pm->policer_templates, pool_index);
        vlib_cli_output (vm, "Name \"%s\" %U ", 
                         name, format_policer_config, config);
        vlib_cli_output (vm, "Template %U", 
                         format_policer_instance, templ);
        vlib_cli_output (vm, "-----------");
      }
  }));
  return 0;
}


VLIB_CLI_COMMAND (show_policer_command, static) = {
    .path = "show policer",
    .short_help = "show policer [name]",
    .function = show_policer_command_fn,
};

clib_error_t *policer_init (vlib_main_t * vm)
{
  vnet_policer_main_t * pm = &vnet_policer_main;
  void vnet_policer_node_funcs_reference(void);

  vnet_policer_node_funcs_reference();
  
  pm->vlib_main = vm;
  pm->vnet_main = vnet_get_main();

  pm->policer_config_by_name = hash_create_string (0, sizeof (uword));
  return 0;
}

VLIB_INIT_FUNCTION(policer_init);


