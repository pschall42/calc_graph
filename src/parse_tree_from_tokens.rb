###################################################
#                     Parsing                     #
###################################################

# y = 5 + x * (3 - 4)
assignment = "5 + x * (3 - 4)"
tokens = [
  {type: :token_number, start: 0, length: 1, line: 1},
  {type: :token_plus, start: 2, length: 1, line: 1},
  {type: :token_identifier, start: 4, length: 1, line: 1},
  {type: :token_star, start: 6, length: 1, line: 1},
  {type: :token_left_paren, start: 8, length: 1, line: 1},
  {type: :token_number, start: 9, length: 1, line: 1},
  {type: :token_minus, start: 11, length: 1, line: 1},
  {type: :token_number, start: 13, length: 1, line: 1},
  {type: :token_right_paren, start: 14, length: 1, line: 1},
  {type: :token_eof, start: 15, length: 0, line: 1}
]

correct_parsed = lambda{
  number_5 = {op: :op_number, value: {number: 5}}
  variable_x = {op: :op_get_variable, value: {variable: :x}}
  number_3 = {op: :op_number, value: {number: 3}}
  number_4 = {op: :op_number, value: {number: 4}}

  sub_3_4 = {op: :op_minus, lhs: number_3, rhs: number_4}
  mul_x_sub_3_4 = {op: :op_mult, lhs: variable_x, rhs: sub_3_4}
  add_5_mul_x_sub_3_4 = {op: :op_add, lhs: number_5, rhs: mul_x_sub_3_4}
}.call()

recursive_descent_parse = lambda{
  scan_token = lambda{|scanner|
    scanner[:current] += 1
    tokens[scanner[:current] - 1]
  }

  error_token = lambda{|scanner|
    {type: :token_error, start: scanner[:start], length: scanner[:current] - scanner[:start], line: 1}
  }

  parser_advance = lambda{|parser|
    #puts parser.inspect
    parser[:previous] = parser[:current]
    parser[:current] = scan_token.call(parser[:scanner])
    parser[:previous]
  }


  parser_peek = lambda{|parser|
    #puts parser.inspect
    parser[:current]
  }

  parser_previous = lambda{|parser|
    #puts parser.inspect
    parser[:previous]
  }

  make_unary_op = lambda{|token, right|
    case token[:type]
    when :token_plus
      {op: :op_add, rhs: right}
    else
      {op: :op_error, rhs: right}
    end
  }

  make_binary_op = lambda{|token, left, right|
    case token[:type]
    when :token_plus
      {op: :op_add, lhs: left, rhs: right}
    when :token_minus
      {op: :op_minus, lhs: left, rhs: right}
    when :token_star
      {op: :op_mult, lhs: left, rhs: right}
    when :token_slash
      {op: :op_div, lhs: left, rhs: right}
    else
      {op: :op_error, lhs: left, rhs: right}
    end
  }

  make_value_op = lambda{|type, value|
    case type
    when :number
      {op: :op_number, value: {number: value}}
    when :variable
      {op: :op_get_variable, value: {variable: value}}
    else
      {op: :op_error, value: {error: value}}
    end
  }

  string_from_token = lambda{|token|
    assignment[token[:start]..(token[:start] + token[:length] - 1)]
  }

  parser_check = lambda{|parser, type|
    #puts parser.inspect
    return false if parser_peek.call(parser)[:type] == :token_eof
    return parser_peek.call(parser)[:type] == type
  }

  parser_match = lambda{|parser, type|
    #puts parser.inspect
    if parser_check.call(parser, type)
      parser_advance.call(parser)
      return true
    end
    return false
  }

  parser_consume = lambda{|parser, type|
    #puts parser.inspect
    if parser_check.call(parser, type)
      return parser_advance.call(parser)
    end
    return error_token.call(parser[:scanner])
  }

  number = lambda{|token|
    value = Float(string_from_token.call(token))
    value = value.to_i if value == value.to_i
    make_value_op.call(:number, value)
  }

  variable = lambda{|token|
    make_value_op.call(:variable, string_from_token.call(token).to_sym)
  }

  #####################################

  primary = lambda{|parser|
    #puts parser.inspect
    if parser_match.call(parser, :token_number)
      return number.call(parser[:previous])
    end
    if parser_match.call(parser, :token_identifier)
      return variable.call(parser[:previous])
    end

    if parser_match.call(parser, :token_left_paren)
      expr = expression.call(parser)
      parser_consume.call(parser, :token_right_paren)
      return expr
    end
  }

  unary = lambda{|parser|
    #puts parser.inspect
    if parser_match.call(parser, :token_minus)
      operator = parser_previous.call(parser)
      right = unary.call(parser)
      return make_unary_op.call(operator, right)
    end
    return primary.call(parser)
  }

  multiplication = lambda{|parser|
    #puts parser.inspect
    expr = unary.call(parser)
    while (parser_match.call(parser, :token_star) || parser_match.call(parser, :token_slash))
      operator = parser_previous.call(parser)
      right = unary.call(parser)
      expr = make_binary_op.call(operator, expr, right)
    end
    expr
  }

  addition = lambda{|parser|
    #puts parser.inspect
    expr = multiplication.call(parser)
    #puts expr.inspect
    #puts "Parser match: "
    #puts parser_check.call(parser, :token_plus)
    while (parser_match.call(parser, :token_plus) || parser_match.call(parser, :token_minus))
      operator = parser_previous.call(parser)
      right = multiplication.call(parser)
      expr = make_binary_op.call(operator, expr, right)
    end
    expr
  }

  expression = lambda{|parser|
    #puts parser.inspect
    addition.call(parser)
  }

  # copy primary again since it needs too capture the expression lambda
  primary = lambda{|parser|
    #puts parser.inspect
    if parser_match.call(parser, :token_number)
      return number.call(parser[:previous])
    end
    if parser_match.call(parser, :token_identifier)
      return variable.call(parser[:previous])
    end

    if parser_match.call(parser, :token_left_paren)
      expr = expression.call(parser)
      parser_consume.call(parser, :token_right_paren)
      return expr
    end
  }

  solver_scanner = {current: 0}
  solver_parser = {scanner: solver_scanner}
  parser_advance.call(solver_parser)
  result = expression.call(solver_parser)

  result
}

#########################################################################################

pratt_parse = lambda{
  scan_token = lambda{|scanner|
    scanner[:current] += 1
    tokens[scanner[:current] - 1]
  }

  error_token = lambda{|scanner|
    {type: :token_error, start: scanner[:start], length: scanner[:current] - scanner[:start], line: 1}
  }

  parser_advance = lambda{|parser|
    #puts parser.inspect
    parser[:previous] = parser[:current]
    parser[:current] = scan_token.call(parser[:scanner])
    parser[:previous]
  }


  parser_peek = lambda{|parser|
    #puts parser.inspect
    parser[:current]
  }

  parser_previous = lambda{|parser|
    #puts parser.inspect
    parser[:previous]
  }

  make_unary_op = lambda{|token, right|
    case token[:type]
    when :token_minus
      {op: :op_negate, rhs: right}
    else
      {op: :op_error, rhs: right}
    end
  }

  make_binary_op = lambda{|token, left, right|
    case token[:type]
    when :token_plus
      {op: :op_add, lhs: left, rhs: right}
    when :token_minus
      {op: :op_minus, lhs: left, rhs: right}
    when :token_star
      {op: :op_mult, lhs: left, rhs: right}
    when :token_slash
      {op: :op_div, lhs: left, rhs: right}
    else
      {op: :op_error, lhs: left, rhs: right}
    end
  }

  make_value_op = lambda{|type, value|
    case type
    when :number
      {op: :op_number, value: {number: value}}
    when :variable
      {op: :op_get_variable, value: {variable: value}}
    else
      {op: :op_error, value: {error: value}}
    end
  }

  string_from_token = lambda{|token|
    assignment[token[:start]..(token[:start] + token[:length] - 1)]
  }

  parser_check = lambda{|parser, type|
    #puts parser.inspect
    return false if parser_peek.call(parser)[:type] == :token_eof
    return parser_peek.call(parser)[:type] == type
  }

  parser_match = lambda{|parser, type|
    #puts parser.inspect
    if parser_check.call(parser, type)
      parser_advance.call(parser)
      return true
    end
    return false
  }

  parser_consume = lambda{|parser, type|
    #puts parser.inspect
    if parser_check.call(parser, type)
      return parser_advance.call(parser)
    end
    return error_token.call(parser[:scanner])
  }

  number = lambda{|token|
    value = Float(string_from_token.call(token))
    value = value.to_i if value == value.to_i
    make_value_op.call(:number, value)
  }

  variable = lambda{|token|
    make_value_op.call(:variable, string_from_token.call(token).to_sym)
  }

  parse_number = lambda{|parser|
    number.call(parser_previous.call(parser))
  }

  parse_variable = lambda{|parser|
    variable.call(parser_previous.call(parser))
  }

  #####################################

  precedences = [
    :prec_none, :prec_assignment, :prec_term,
    :prec_factor, :prec_exponent, :prec_unary,
    :prec_call, :prec_primary
  ]

  # declare early since it needs to capture the get_rule lambda
  parse_rules_constant = {}

  get_rule = lambda{|token_type|
    parse_rules_constant[token_type]
  }

  parse_precedence = lambda{|parser, precedence|
    parser_advance.call(parser)
    prec_idx = precedences.index(precedence)

    prefix_rule = get_rule.call(parser_previous.call(parser)[:type])[:prefix]
    if (prefix_rule.nil?)
      return error_token.call(parser[:scanner])
    end
    expr = prefix_rule.call(parser)

    while (prec_idx <= precedences.index(get_rule.call(parser_peek.call(parser)[:type])[:precedence]))
      parser_advance.call(parser)
      infix_rule = get_rule.call(parser_previous.call(parser)[:type])[:infix]
      expr = infix_rule.call(parser, expr)
    end

    return expr
  }

  expression = lambda{|parser|
    parse_precedence.call(parser, :prec_assignment)
  }

  grouping = lambda{|parser|
    expr = expression.call(parser)
    parser_consume.call(parser, :token_right_paren)
    expr
  }

  unary = lambda{|parser|
    # remember the operator
    op_type = parser_previous.call(parser)[:type]

    # parse operand
    right = parse_precedence.call(parser, :prec_unary)

    make_unary_op.call(op_type, right)
  }

  binary = lambda{|parser, left|
    # remember the operator
    op = parser_previous.call(parser)

    # get the parse rule and evaluate the right operand
    rule = get_rule.call(op[:type])
    next_precedence = precedences[precedences.index(rule[:precedence]) + 1]

    right = parse_precedence.call(parser, next_precedence)

    make_binary_op.call(op, left, right)
  }






  parse_rules_constant = {
    token_identifier:  {prefix: parse_variable, infix: nil,    precedence: :prec_none},
    token_number:      {prefix: parse_number,   infix: nil,    precedence: :prec_none},
    token_plus:        {prefix: nil,            infix: binary, precedence: :prec_term},
    token_minus:       {prefix: unary,          infix: binary, precedence: :prec_term},
    token_star:        {prefix: nil,            infix: binary, precedence: :prec_factor},
    token_slash:       {prefix: nil,            infix: binary, precedence: :prec_factor},
    token_percent:     {prefix: nil,            infix: binary, precedence: :prec_factor},
    token_carat:       {prefix: nil,            infix: binary, precedence: :prec_exponent},
    token_left_paren:  {prefix: grouping,       infix: nil,    precedence: :prec_none},
    token_right_paren: {prefix: nil,            infix: nil,    precedence: :prec_none},
    token_eof:         {prefix: nil,            infix: nil,    precedence: :prec_none}
  }

  solver_scanner = {current: 0}
  solver_parser = {scanner: solver_scanner}
  parser_advance.call(solver_parser)
  result = expression.call(solver_parser)

  result
}


rd_result = recursive_descent_parse.call()
rd_result == correct_parsed


pratt_result = pratt_parse.call()
pratt_result == correct_parsed


##################################################################################################

###################################################
#                  Topology Sort                  #
###################################################

equation_trees = lambda{
  number = lambda{|num| {op: :op_number, value: {number: num}} }
  variable = lambda{|var| {op: :op_get_variable, value: {variable: var}} }
  binary = lambda{|op, left, right| {op: op, left: left, right: right} }
  unary = lambda {|op, right| {op: op, right: right} }
  add = lambda{|left, right| binary.call(:op_add, left, right) }
  sub = lambda{|left, right| binary.call(:op_minus, left, right) }
  mul = lambda{|left, right| binary.call(:op_mult, left, right) }
  div = lambda{|left, right| binary.call(:op_div, left, right) }

  ## Inputs: t, u
  # y = 5 + x * (3 - 4)
  y = add.call(number.call(5), mul.call(variable.call(:x), sub.call(number.call(3), number.call(4))))
  # x = 4 * t(input)
  x = mul.call(number.call(4), variable.call(:t))
  # z = u(input) - x * y
  z = sub.call(variable.call(:u), mul.call(variable.call(:x), variable.call(:y)))
  # v = w - x
  v = sub.call(variable.call(:w), variable.call(:x))
  # w = u(input) / 5 + t(input)
  w = add.call(div.call(variable.call(:u), number.call(5)), variable.call(:t))

  l1 = add.call(variable.call(:w), variable.call(:l2))
  l2 = sub.call(variable.call(:x), variable.call(:l3))
  l3 = mul.call(variable.call(:y), variable.call(:l4))
  l4 = div.call(variable.call(:z), variable.call(:l1))

  #trees = {y: y, x: x, z: z, v: v, w: w, l1: l1, l2: l2, l3: l3, l4: l4}
  trees = {y: y, x: x, z: z, v: v, w: w}
}.call()

correct_depths = {t: 0, u: 0, v: 2, w: 1, x: 1, y: 2, z: 3}
correct_topology = [[:t, :u], [:w, :x], [:v, :y], [:z]]

# this is only relevant as a solver, when doing update semantics, topological sorting doesn't matter
# because we're mapping things over time, so the original values are all inputs
topological_sort = lambda{
  max = lambda{|x, y| x > y ? x : y}
  top_sort = lambda{|syntax_trees_hash, current_var, current_node, var_depths_hash, topology, traversed_vars| } # forward declaration
  max_depth = lambda{|syntax_trees_hash, current_node, var_depths_hash, topology, traversed_vars|
    left_depth = 0
    right_depth = 0
    value_depth = 0
    #puts "Max Depth: current_node: #{current_node.inspect}"
    if current_node[:left] != nil
      left_depth = max_depth.call(syntax_trees_hash, current_node[:left], var_depths_hash, topology, traversed_vars)
    end
    if current_node[:right] != nil
      right_depth = max_depth.call(syntax_trees_hash, current_node[:right], var_depths_hash, topology, traversed_vars)
    end
    if current_node[:op] == :op_get_variable && current_node[:value] != nil
      var = current_node[:value][:variable]
      if var_depths_hash[var] != nil
        return var_depths_hash[var] + 1
      end
      var_tree = syntax_trees_hash[var]
      # var is an input
      if var_tree == nil
        var_depths_hash[var] = 0
        if topology.size == 0
          topology << []
        end
        topology[0] << var
        return 1
      end
      value_depth = top_sort.call(syntax_trees_hash, var, var_tree, var_depths_hash, topology, traversed_vars) + 1
    end
    #puts "Max Depth: left_depth: #{left_depth.inspect}"
    #puts "Max Depth: right_depth: #{right_depth.inspect}"
    #puts "Max Depth: value_depth: #{value_depth.inspect}"

    return max.call(max.call(left_depth, right_depth), value_depth)
  }
  top_sort = lambda{|syntax_trees_hash, current_var, current_node, var_depths_hash, topology, traversed_vars|
    #puts "Top Sort: current_var: #{current_var.inspect}"
    #puts "Top Sort: current_node: #{current_node.inspect}"
    if traversed_vars.include?(current_var)
      raise "Loop detected"
    end
    traversed_vars << current_var
    if var_depths_hash[current_var] != nil
      return var_depths_hash[current_var]
    end
    depth = max_depth.call(syntax_trees_hash, current_node, var_depths_hash, topology, traversed_vars)

    var_depths_hash[current_var] = depth
    if topology.size <= depth # need to expand array
      topology << []
    end
    topology[depth] << current_var
    return depth
  }

  var_depths = {}
  topology = []
  for var, tree in equation_trees do
    top_sort.call(equation_trees, var, tree, var_depths, topology, [])
  end
  return {var_depths: var_depths, topology: topology}
}
compare_var_depths = lambda{|h1, h2|
  equal = true
  h1.each do |k,v|
    equal = equal && h2[k] == v
  end
  h2.each do |k,v|
    equal = equal && h1[k] == v
  end
  return equal
}
compare_topologies = lambda{|a1, a2|
  equal = true
  a1.each_with_index do |vars, idx|
    equal = equal && a2[idx].map{|var| vars.include?(var)}.all? && vars.map{|var| a2[idx].include?(var)}.all?
  end
  a2.each_with_index do |vars, idx|
    equal = equal && a1[idx].map{|var| vars.include?(var)}.all? && vars.map{|var| a1[idx].include?(var)}.all?
  end
  return equal
}

top_sort_results = topological_sort.call()
depths_correct = compare_var_depths.call(correct_depths, top_sort_results[:var_depths])
tops_correct = compare_topologies.call(correct_topology, top_sort_results[:topology])
depths_correct && tops_correct


##################################################################################################

###################################################
#               Code Gen + Stack Vm               #
###################################################
generate_stack_code = lambda {|ext_consts = [], ext_defaults = {}, ext_bytecode = []|
  get_constant_index = lambda {|constants, number_val|
    if constants.include?(number_val)
      return constants.index(number_val)
    end
    constants << number_val
    return constants.size - 1
  }

  bytecode_from_tree = lambda{|current_node, constants|
    #puts "bytecode_from_tree: current_node: #{current_node.inspect}"
    #puts "bytecode_from_tree: constants: #{constants.inspect}"
    if current_node[:op] == :op_get_variable && current_node[:value] != nil
      var_lookup = current_node[:value][:variable]
      return [:op_get_variable, var_lookup]
    end
    if current_node[:op] == :op_number && current_node[:value] != nil
      number_val = current_node[:value][:number]
      number_lookup = get_constant_index.call(constants, number_val)
      return [:op_number, number_lookup]
    end

    left_bytecode = []
    right_bytecode = []
    if current_node[:left] != nil
      left_bytecode = bytecode_from_tree.call(current_node[:left], constants)
    end
    if current_node[:right] != nil
      right_bytecode = bytecode_from_tree.call(current_node[:right], constants)
    end
    return left_bytecode + right_bytecode + [current_node[:op]]
  }

  bytecode_from_topology = lambda{|equations, topology, constants, bytecode|
    for vars in topology do
      for var in vars do
        current_tree = equations[var]
        # current_tree is only nil if the variable is an input
        if current_tree != nil
          #puts "bytecode_from_topology: equations: #{equations.inspect}"
          #puts "bytecode_from_topology: var: #{var.inspect}"
          current_bytecode = bytecode_from_tree.call(current_tree, constants)
          (current_bytecode + [:op_set_variable, var]).each do |byte|
            bytecode << byte
          end
        end
      end
    end
    return bytecode
  }

  bytecode_from_topology.call(equation_trees, top_sort_results[:topology], ext_consts, ext_bytecode)

  # Set some basic defaults
  ext_defaults[:t] = 0
  ext_defaults[:u] = 0
}

stack_vm = lambda {|input_vars|

  init_vm = lambda{|vm, constants, variables, defaults, bytecode|
    vm[:ip] = 0
    vm[:stack] = Array.new(1)
    vm[:stack_top] = 0
    vm[:stack_size] = 1
    vm[:constants] = constants
    vm[:variables] = variables
    vm[:defaults] = defaults
    vm[:bytecode] = bytecode
  }

  read_byte = lambda{|vm|
    vm[:ip] += 1
    return vm[:bytecode][vm[:ip] - 1]
  }

  grow_stack = lambda{|vm|
    vm[:stack_size].times do
      vm[:stack] << nil
    end
    vm[:stack_size] *= 2
  }

  stack_push = lambda{|vm, value|
    idx = vm[:stack_top]
    if idx == vm[:stack_size]
      grow_stack.call(vm)
    end
    vm[:stack][idx] = value
    vm[:stack_top] += 1
    return nil
  }

  stack_pop = lambda{|vm|
    idx = vm[:stack_top] - 1
    vm[:stack_top] = idx
    return vm[:stack][idx]
  }

  process_operation = lambda{|vm, opcode|
    #puts "process_operation: opcode: #{opcode.inspect}"
    #puts "process_operation: stack before: #{vm[:stack].inspect}"
    #puts "process_operation: variables before: #{vm[:variables].inspect}"
    case opcode
    when :op_number
      num_addr = read_byte.call(vm)
      num_val = vm[:constants][num_addr]
      stack_push.call(vm, num_val)
    when :op_get_variable
      var_addr = read_byte.call(vm)
      var_val = vm[:variables][var_addr]
      if var_val == nil
        var_val = vm[:defaults][var_addr]
        vm[:variables][var_addr] = var_val
      end
      stack_push.call(vm, var_val)
    when :op_set_variable
      var_addr = read_byte.call(vm)
      var_val = stack_pop.call(vm)
      vm[:variables][var_addr] = var_val
    when :op_add
      val_2 = stack_pop.call(vm)
      val_1 = stack_pop.call(vm)
      stack_push.call(vm, val_1 + val_2)
    when :op_minus
      val_2 = stack_pop.call(vm)
      val_1 = stack_pop.call(vm)
      stack_push.call(vm, val_1 - val_2)
    when :op_mult
      val_2 = stack_pop.call(vm)
      val_1 = stack_pop.call(vm)
      stack_push.call(vm, val_1 * val_2)
    when :op_div
      val_2 = stack_pop.call(vm)
      val_1 = stack_pop.call(vm)
      stack_push.call(vm, val_1 / Float(val_2))
    end
    #puts "process_operation: stack after: #{vm[:stack].inspect}"
    #puts "process_operation: variables after: #{vm[:variables].inspect}"
  }

  run_vm = lambda{|vm|
    while vm[:ip] < vm[:bytecode].size
      opcode = read_byte.call(vm)
      process_operation.call(vm, opcode)
    end
  }

  final_variables = lambda{|vm|
    return vm[:variables].dup
  }

  vm = {}
  constants = []
  variables = input_vars.dup
  defaults = {}
  bytecode = []
  generate_stack_code.call(constants, defaults, bytecode)
  init_vm.call(vm, constants, variables, defaults, bytecode)
  puts "Initial VM: #{vm.inspect}"

  run_vm.call(vm)

  return final_variables.call(vm)
}

stack_vm.call({})

##################################################################################################

###################################################
#             Code Gen + Register Vm              #
###################################################


generate_register_code = lambda {|ext_reg_count = 2, ext_consts = [], ext_defaults = {}, ext_bytecode = []|
  get_constant_index = lambda {|constants, number_val|
    if constants.include?(number_val)
      return constants.index(number_val)
    end
    constants << number_val
    return constants.size - 1
  }

  # TODO: finish
  get_register_idx = lambda {|reg_count, |}

  # TODO: finish
  bytecode_from_tree = lambda{|current_node, constants|
    #puts "bytecode_from_tree: current_node: #{current_node.inspect}"
    #puts "bytecode_from_tree: constants: #{constants.inspect}"
    if current_node[:op] == :op_get_variable && current_node[:value] != nil
      var_lookup = current_node[:value][:variable]
      return [:op_get_variable, var_lookup]
    end
    if current_node[:op] == :op_number && current_node[:value] != nil
      number_val = current_node[:value][:number]
      register = get_register_idx
      number_lookup = get_constant_index.call(constants, number_val)
      return [:op_number, register, number_lookup]
    end

    left_bytecode = []
    right_bytecode = []
    if current_node[:left] != nil
      left_bytecode = bytecode_from_tree.call(current_node[:left], constants)
    end
    if current_node[:right] != nil
      right_bytecode = bytecode_from_tree.call(current_node[:right], constants)
    end
    return left_bytecode + right_bytecode + [current_node[:op]]
  }

  bytecode_from_topology = lambda{|equations, topology, constants, bytecode|
    for vars in topology do
      for var in vars do
        current_tree = equations[var]
        # current_tree is only nil if the variable is an input
        if current_tree != nil
          #puts "bytecode_from_topology: equations: #{equations.inspect}"
          #puts "bytecode_from_topology: var: #{var.inspect}"
          current_bytecode = bytecode_from_tree.call(current_tree, constants)
          (current_bytecode + [:op_set_variable, var]).each do |byte|
            bytecode << byte
          end
        end
      end
    end
    return bytecode
  }

  bytecode_from_topology.call(equation_trees, top_sort_results[:topology], ext_consts, ext_bytecode)

  # Set some basic defaults
  ext_defaults[:t] = 0
  ext_defaults[:u] = 0
}

register_vm = lambda {|input_vars|

  init_vm = lambda{|vm, constants, variables, defaults, bytecode|
    vm[:ip] = 0
    vm[:registers] = Array.new(1)
    vm[:register_size] = 1
    vm[:constants] = constants
    vm[:variables] = variables
    vm[:defaults] = defaults
    vm[:bytecode] = bytecode
  }

  read_byte = lambda{|vm|
    vm[:ip] += 1
    return vm[:bytecode][vm[:ip] - 1]
  }

  add_registers = lambda{|vm, num_registers|
    num_registers.times do
      vm[:registers] << nil
    end
    vm[:register_size] += num_registers
  }

  register_write = lambda{|vm, reg, value|
    if reg >= vm[:register_size]
      add_registers.call(vm, (reg + 1) - vm[:register_size])
    end
    vm[:registers][reg] = value
  }
  register_read = lambda{|vm, reg|
    return vm[:registers][reg]
  }

  process_operation = lambda{|vm, opcode|
    #puts "process_operation: opcode: #{opcode.inspect}"
    #puts "process_operation: stack before: #{vm[:stack].inspect}"
    #puts "process_operation: variables before: #{vm[:variables].inspect}"
    case opcode
    when :op_number
      dst_register = read_byte.call()
      num_addr = read_byte.call(vm)
      num_val = vm[:constants][num_addr]
      register_write.call(vm, dst_register, num_val)
    when :op_get_variable
      dst_register = read_byte.call()
      var_addr = read_byte.call(vm)
      var_val = vm[:variables][var_addr]
      if var_val == nil
        var_val = vm[:defaults][var_addr]
        vm[:variables][var_addr] = var_val
      end
      register_write.call(vm, dst_register, var_val)
    when :op_set_variable
      var_addr = read_byte.call(vm)
      src_register = read_byte.call(vm)
      var_val = register_read.call(vm, reg)
      vm[:variables][var_addr] = var_val
    when :op_add
      dst_register = read_byte.call()
      src_reg_left = read_byte.call()
      src_reg_right = read_byte.call()

      val_left = register_read.call(vm, src_reg_left)
      val_right = register_read.call(vm, src_reg_right)
      register_write.call(vm, dst_register, val_left + val_right)
    when :op_minus
      dst_register = read_byte.call()
      src_reg_left = read_byte.call()
      src_reg_right = read_byte.call()

      val_left = register_read.call(vm, src_reg_left)
      val_right = register_read.call(vm, src_reg_right)
      register_write.call(vm, dst_register, val_left - val_right)
    when :op_mult
      dst_register = read_byte.call()
      src_reg_left = read_byte.call()
      src_reg_right = read_byte.call()

      val_left = register_read.call(vm, src_reg_left)
      val_right = register_read.call(vm, src_reg_right)
      register_write.call(vm, dst_register, val_left * val_right)
    when :op_div
      dst_register = read_byte.call()
      src_reg_left = read_byte.call()
      src_reg_right = read_byte.call()

      val_left = register_read.call(vm, src_reg_left)
      val_right = register_read.call(vm, src_reg_right)
      register_write.call(vm, dst_register, val_left / val_right)
    end
    #puts "process_operation: stack after: #{vm[:stack].inspect}"
    #puts "process_operation: variables after: #{vm[:variables].inspect}"
  }

  run_vm = lambda{|vm|
    while vm[:ip] < vm[:bytecode].size
      opcode = read_byte.call(vm)
      process_operation.call(vm, opcode)
    end
  }

  final_variables = lambda{|vm|
    return vm[:variables].dup
  }

  vm = {}
  constants = []
  variables = input_vars.dup
  defaults = {}
  bytecode = []
  generate_register_code.call(2, constants, defaults, bytecode)
  init_vm.call(vm, constants, variables, defaults, bytecode)
  puts "Initial VM: #{vm.inspect}"

  run_vm.call(vm)

  return final_variables.call(vm)
}

stack_vm.call({})
