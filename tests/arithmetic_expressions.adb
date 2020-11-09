with memory;
use memory;

package body arithmetic_expressions is

   ---------------
   -- create_id --
   ---------------

    function create_id (ch: in character) return id is
        var: id;
    begin
        var.ch := ch;
        return var;
   end create_id;

   ---------------------------
   -- create_var_expression --
   ---------------------------

    function create_var_expression  (var: in id) return arithmetic_expression_access  is
        expr: arithmetic_expression_access;
    begin
        expr := new arithmetic_expression (ID_TYPE);
        expr.var := var;
        return expr;
   end create_var_expression;

   ----------------------------
   -- create_literal_integer --
   ----------------------------

    function create_literal_integer  (value: in integer) return literal_integer is
        li: literal_integer;
    begin
        li.value := value;
        return li;
   end create_literal_integer;

   --------------------------------
   -- create_constant_expression --
   --------------------------------

   function create_constant_expression  (li: in literal_integer) return arithmetic_expression_access
    is
        expr: arithmetic_expression_access;
    begin
        expr := new arithmetic_expression(LITERAL_TYPE);
        expr.li := li;
        return expr;
   end create_constant_expression;

   ------------------------------
   -- create_binary_expression --
   ------------------------------

    function create_binary_expression (op: in arithmetic_operator;
                                       expr1, expr2: in arithmetic_expression_access)
                                       return arithmetic_expression_access is
        expr: arithmetic_expression_access;
    begin
        expr := new arithmetic_expression(BINARY_TYPE);
        expr.op := op;
        expr.expr1 := expr1;
        expr.expr2 := expr2;
        return expr;
   end create_binary_expression;

   --------------
   -- get_char --
   --------------

   function get_char (var: in id) return character is
   begin
        return var.ch;
   end get_char;

   --------------
   -- evaluate --
   --------------

    function evaluate (expr: in arithmetic_expression_access) return integer is
        value: integer;
    begin
        case expr.expr_type is
            when LITERAL_TYPE =>
                value := expr.li.value;
            when ID_TYPE =>
                value := fetch (expr.var.ch);
            when BINARY_TYPE =>
                case expr.op is
                     when ADD_OP =>
                         value := evaluate(expr.expr1) + evaluate (expr.expr2);
                     when SUB_OP =>
                         value := evaluate(expr.expr1) - evaluate (expr.expr2);
                     when MUL_OP =>
                         value := evaluate(expr.expr1) * evaluate (expr.expr2);
                     when DIV_OP =>
                         value := evaluate(expr.expr1) / evaluate (expr.expr2);
                 end case;
        end case;
        return value;
   end evaluate;

end arithmetic_expressions;
