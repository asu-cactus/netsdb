©¿
¤õ
^
AssignVariableOp
resource
value"dtype"
dtypetype"
validate_shapebool( 
8
Const
output"dtype"
valuetensor"
dtypetype
.
Identity

input"T
output"T"	
Ttype
e
MergeV2Checkpoints
checkpoint_prefixes
destination_prefix"
delete_old_dirsbool(

NoOp
M
Pack
values"T*N
output"T"
Nint(0"	
Ttype"
axisint 
³
PartitionedCall
args2Tin
output2Tout"
Tin
list(type)("
Tout
list(type)("	
ffunc"
configstring "
config_protostring "
executor_typestring 
C
Placeholder
output"dtype"
dtypetype"
shapeshape:
@
ReadVariableOp
resource
value"dtype"
dtypetype
o
	RestoreV2

prefix
tensor_names
shape_and_slices
tensors2dtypes"
dtypes
list(type)(0
l
SaveV2

prefix
tensor_names
shape_and_slices
tensors2dtypes"
dtypes
list(type)(0
?
Select
	condition

t"T
e"T
output"T"	
Ttype
H
ShardedFilename
basename	
shard

num_shards
filename
f
SimpleMLCreateModelResource
model_handle"
	containerstring "
shared_namestring 
á
SimpleMLInferenceOpWithHandle
numerical_features
boolean_features
categorical_int_features'
#categorical_set_int_features_values1
-categorical_set_int_features_row_splits_dim_1	1
-categorical_set_int_features_row_splits_dim_2	
model_handle
dense_predictions
dense_col_representation"
dense_output_dimint(0
f
#SimpleMLLoadModelFromPathWithHandle
model_handle
path" 
output_typeslist(string)
 
Á
StatefulPartitionedCall
args2Tin
output2Tout"
Tin
list(type)("
Tout
list(type)("	
ffunc"
configstring "
config_protostring "
executor_typestring ¨
@
StaticRegexFullMatch	
input

output
"
patternstring
m
StaticRegexReplace	
input

output"
patternstring"
rewritestring"
replace_globalbool(
ö
StridedSlice

input"T
begin"Index
end"Index
strides"Index
output"T"	
Ttype"
Indextype:
2	"

begin_maskint "
end_maskint "
ellipsis_maskint "
new_axis_maskint "
shrink_axis_maskint 
N

StringJoin
inputs*N

output"
Nint(0"
	separatorstring 

VarHandleOp
resource"
	containerstring "
shared_namestring "
dtypetype"
shapeshape"#
allowed_deviceslist(string)
 
9
VarIsInitializedOp
resource
is_initialized
"serve*2.8.02v2.8.0-rc1-32-g3f878cff5b68íã
h

is_trainedVarHandleOp*
_output_shapes
: *
dtype0
*
shape: *
shared_name
is_trained
a
is_trained/Read/ReadVariableOpReadVariableOp
is_trained*
_output_shapes
: *
dtype0


SimpleMLCreateModelResourceSimpleMLCreateModelResource*
_output_shapes
: *E
shared_name64simple_ml_model_355c76c8-ee88-40e5-a8ab-d79fd5fd888a
W
asset_path_initializerPlaceholder*
_output_shapes
: *
dtype0*
shape: 

VariableVarHandleOp*
_class
loc:@Variable*
_output_shapes
: *
dtype0*
shape: *
shared_name
Variable
a
)Variable/IsInitialized/VarIsInitializedOpVarIsInitializedOpVariable*
_output_shapes
: 
R
Variable/AssignAssignVariableOpVariableasset_path_initializer*
dtype0
]
Variable/Read/ReadVariableOpReadVariableOpVariable*
_output_shapes
: *
dtype0
Y
asset_path_initializer_1Placeholder*
_output_shapes
: *
dtype0*
shape: 


Variable_1VarHandleOp*
_class
loc:@Variable_1*
_output_shapes
: *
dtype0*
shape: *
shared_name
Variable_1
e
+Variable_1/IsInitialized/VarIsInitializedOpVarIsInitializedOp
Variable_1*
_output_shapes
: 
X
Variable_1/AssignAssignVariableOp
Variable_1asset_path_initializer_1*
dtype0
a
Variable_1/Read/ReadVariableOpReadVariableOp
Variable_1*
_output_shapes
: *
dtype0
Y
asset_path_initializer_2Placeholder*
_output_shapes
: *
dtype0*
shape: 


Variable_2VarHandleOp*
_class
loc:@Variable_2*
_output_shapes
: *
dtype0*
shape: *
shared_name
Variable_2
e
+Variable_2/IsInitialized/VarIsInitializedOpVarIsInitializedOp
Variable_2*
_output_shapes
: 
X
Variable_2/AssignAssignVariableOp
Variable_2asset_path_initializer_2*
dtype0
a
Variable_2/Read/ReadVariableOpReadVariableOp
Variable_2*
_output_shapes
: *
dtype0
Y
asset_path_initializer_3Placeholder*
_output_shapes
: *
dtype0*
shape: 


Variable_3VarHandleOp*
_class
loc:@Variable_3*
_output_shapes
: *
dtype0*
shape: *
shared_name
Variable_3
e
+Variable_3/IsInitialized/VarIsInitializedOpVarIsInitializedOp
Variable_3*
_output_shapes
: 
X
Variable_3/AssignAssignVariableOp
Variable_3asset_path_initializer_3*
dtype0
a
Variable_3/Read/ReadVariableOpReadVariableOp
Variable_3*
_output_shapes
: *
dtype0
Y
asset_path_initializer_4Placeholder*
_output_shapes
: *
dtype0*
shape: 


Variable_4VarHandleOp*
_class
loc:@Variable_4*
_output_shapes
: *
dtype0*
shape: *
shared_name
Variable_4
e
+Variable_4/IsInitialized/VarIsInitializedOpVarIsInitializedOp
Variable_4*
_output_shapes
: 
X
Variable_4/AssignAssignVariableOp
Variable_4asset_path_initializer_4*
dtype0
a
Variable_4/Read/ReadVariableOpReadVariableOp
Variable_4*
_output_shapes
: *
dtype0
a
ReadVariableOpReadVariableOpVariable^Variable/Assign*
_output_shapes
: *
dtype0
ª
StatefulPartitionedCallStatefulPartitionedCallReadVariableOpSimpleMLCreateModelResource*
Tin
2*
Tout
2*
_collective_manager_ids
 *
_output_shapes
: * 
_read_only_resource_inputs
 *-
config_proto

CPU

GPU 2J 8 *#
fR
__inference_<lambda>_17017

NoOpNoOp^StatefulPartitionedCall^Variable/Assign^Variable_1/Assign^Variable_2/Assign^Variable_3/Assign^Variable_4/Assign
­

ConstConst"/device:CPU:0*
_output_shapes
: *
dtype0*è	
valueÞ	BÛ	 BÔ	
õ
_learner_params
	_features
_is_trained
	optimizer
loss

_model
	variables
trainable_variables
	regularization_losses

	keras_api
__call__
*&call_and_return_all_conditional_losses
_default_save_signature
_build_normalized_inputs
call
call_get_leaves
yggdrasil_model_path_tensor

signatures*
* 
* 
JD
VARIABLE_VALUE
is_trained&_is_trained/.ATTRIBUTES/VARIABLE_VALUE*
* 
* 
+
_input_builder
_compiled_model* 

0*
* 
* 
°
non_trainable_variables

layers
metrics
layer_regularization_losses
layer_metrics
	variables
trainable_variables
	regularization_losses
__call__
_default_save_signature
*&call_and_return_all_conditional_losses
&"call_and_return_conditional_losses*
* 
* 
* 
* 
* 
* 
* 

serving_default* 
P
_feature_name_to_idx
	_init_ops
#categorical_str_to_int_hashmaps* 
S
_model_loader
_create_resource
 _initialize
!_destroy_resource* 

0*
* 
* 
* 
* 
* 
* 
* 
* 
5
"_output_types
#
_all_files
$
_done_file* 
* 
* 
* 
* 
%
%0
&1
'2
$3
(4* 
* 
* 
* 
* 
* 
s
serving_default_jet1btagPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
r
serving_default_jet1etaPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
r
serving_default_jet1phiPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
q
serving_default_jet1ptPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
s
serving_default_jet2btagPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
r
serving_default_jet2etaPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
r
serving_default_jet2phiPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
q
serving_default_jet2ptPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
s
serving_default_jet3btagPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
r
serving_default_jet3etaPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
r
serving_default_jet3phiPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
q
serving_default_jet3ptPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
s
serving_default_jet4btagPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
r
serving_default_jet4etaPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
r
serving_default_jet4phiPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
q
serving_default_jet4ptPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
t
serving_default_leptonetaPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
t
serving_default_leptonphiPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
s
serving_default_leptonptPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
n
serving_default_mbbPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ

&serving_default_missingenergymagnitudePlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
{
 serving_default_missingenergyphiPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
n
serving_default_mjjPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
o
serving_default_mjjjPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
o
serving_default_mjlvPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
n
serving_default_mlvPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
o
serving_default_mwbbPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ
p
serving_default_mwwbbPlaceholder*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*
dtype0*
shape:ÿÿÿÿÿÿÿÿÿ

StatefulPartitionedCall_1StatefulPartitionedCallserving_default_jet1btagserving_default_jet1etaserving_default_jet1phiserving_default_jet1ptserving_default_jet2btagserving_default_jet2etaserving_default_jet2phiserving_default_jet2ptserving_default_jet3btagserving_default_jet3etaserving_default_jet3phiserving_default_jet3ptserving_default_jet4btagserving_default_jet4etaserving_default_jet4phiserving_default_jet4ptserving_default_leptonetaserving_default_leptonphiserving_default_leptonptserving_default_mbb&serving_default_missingenergymagnitude serving_default_missingenergyphiserving_default_mjjserving_default_mjjjserving_default_mjlvserving_default_mlvserving_default_mwbbserving_default_mwwbbSimpleMLCreateModelResource*(
Tin!
2*
Tout
2*
_collective_manager_ids
 *'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ* 
_read_only_resource_inputs
 *-
config_proto

CPU

GPU 2J 8 *,
f'R%
#__inference_signature_wrapper_16991
O
saver_filenamePlaceholder*
_output_shapes
: *
dtype0*
shape: 
»
StatefulPartitionedCall_2StatefulPartitionedCallsaver_filenameis_trained/Read/ReadVariableOpConst*
Tin
2
*
Tout
2*
_collective_manager_ids
 *
_output_shapes
: * 
_read_only_resource_inputs
 *-
config_proto

CPU

GPU 2J 8 *'
f"R 
__inference__traced_save_17093
¢
StatefulPartitionedCall_3StatefulPartitionedCallsaver_filename
is_trained*
Tin
2*
Tout
2*
_collective_manager_ids
 *
_output_shapes
: * 
_read_only_resource_inputs
 *-
config_proto

CPU

GPU 2J 8 **
f%R#
!__inference__traced_restore_17106
ß9
²
*__inference__build_normalized_inputs_16877
inputs_jet1btag
inputs_jet1eta
inputs_jet1phi
inputs_jet1pt
inputs_jet2btag
inputs_jet2eta
inputs_jet2phi
inputs_jet2pt
inputs_jet3btag
inputs_jet3eta
inputs_jet3phi
inputs_jet3pt
inputs_jet4btag
inputs_jet4eta
inputs_jet4phi
inputs_jet4pt
inputs_leptoneta
inputs_leptonphi
inputs_leptonpt

inputs_mbb!
inputs_missingenergymagnitude
inputs_missingenergyphi

inputs_mjj
inputs_mjjj
inputs_mjlv

inputs_mlv
inputs_mwbb
inputs_mwwbb
identity

identity_1

identity_2

identity_3

identity_4

identity_5

identity_6

identity_7

identity_8

identity_9
identity_10
identity_11
identity_12
identity_13
identity_14
identity_15
identity_16
identity_17
identity_18
identity_19
identity_20
identity_21
identity_22
identity_23
identity_24
identity_25
identity_26
identity_27S
IdentityIdentityinputs_jet1btag*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿT

Identity_1Identityinputs_jet1eta*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿT

Identity_2Identityinputs_jet1phi*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿS

Identity_3Identityinputs_jet1pt*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿU

Identity_4Identityinputs_jet2btag*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿT

Identity_5Identityinputs_jet2eta*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿT

Identity_6Identityinputs_jet2phi*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿS

Identity_7Identityinputs_jet2pt*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿU

Identity_8Identityinputs_jet3btag*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿT

Identity_9Identityinputs_jet3eta*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿU
Identity_10Identityinputs_jet3phi*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿT
Identity_11Identityinputs_jet3pt*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿV
Identity_12Identityinputs_jet4btag*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿU
Identity_13Identityinputs_jet4eta*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿU
Identity_14Identityinputs_jet4phi*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿT
Identity_15Identityinputs_jet4pt*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿW
Identity_16Identityinputs_leptoneta*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿW
Identity_17Identityinputs_leptonphi*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿV
Identity_18Identityinputs_leptonpt*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿQ
Identity_19Identity
inputs_mbb*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿd
Identity_20Identityinputs_missingenergymagnitude*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ^
Identity_21Identityinputs_missingenergyphi*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿQ
Identity_22Identity
inputs_mjj*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿR
Identity_23Identityinputs_mjjj*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿR
Identity_24Identityinputs_mjlv*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿQ
Identity_25Identity
inputs_mlv*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿR
Identity_26Identityinputs_mwbb*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿS
Identity_27Identityinputs_mwwbb*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ"
identityIdentity:output:0"!

identity_1Identity_1:output:0"#
identity_10Identity_10:output:0"#
identity_11Identity_11:output:0"#
identity_12Identity_12:output:0"#
identity_13Identity_13:output:0"#
identity_14Identity_14:output:0"#
identity_15Identity_15:output:0"#
identity_16Identity_16:output:0"#
identity_17Identity_17:output:0"#
identity_18Identity_18:output:0"#
identity_19Identity_19:output:0"!

identity_2Identity_2:output:0"#
identity_20Identity_20:output:0"#
identity_21Identity_21:output:0"#
identity_22Identity_22:output:0"#
identity_23Identity_23:output:0"#
identity_24Identity_24:output:0"#
identity_25Identity_25:output:0"#
identity_26Identity_26:output:0"#
identity_27Identity_27:output:0"!

identity_3Identity_3:output:0"!

identity_4Identity_4:output:0"!

identity_5Identity_5:output:0"!

identity_6Identity_6:output:0"!

identity_7Identity_7:output:0"!

identity_8Identity_8:output:0"!

identity_9Identity_9:output:0*(
_construction_contextkEagerRuntime*¹
_input_shapes§
¤:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:T P
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet1btag:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet1eta:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet1phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet1pt:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet2btag:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet2eta:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet2phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet2pt:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet3btag:S	O
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet3eta:S
O
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet3phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet3pt:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet4btag:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet4eta:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet4phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet4pt:UQ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
*
_user_specified_nameinputs/leptoneta:UQ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
*
_user_specified_nameinputs/leptonphi:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/leptonpt:OK
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
$
_user_specified_name
inputs/mbb:b^
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
7
_user_specified_nameinputs/missingenergymagnitude:\X
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
1
_user_specified_nameinputs/missingenergyphi:OK
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
$
_user_specified_name
inputs/mjj:PL
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
%
_user_specified_nameinputs/mjjj:PL
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
%
_user_specified_nameinputs/mjlv:OK
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
$
_user_specified_name
inputs/mlv:PL
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
%
_user_specified_nameinputs/mwbb:QM
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
&
_user_specified_nameinputs/mwwbb
»7

__inference_call_16949
inputs_jet1btag
inputs_jet1eta
inputs_jet1phi
inputs_jet1pt
inputs_jet2btag
inputs_jet2eta
inputs_jet2phi
inputs_jet2pt
inputs_jet3btag
inputs_jet3eta
inputs_jet3phi
inputs_jet3pt
inputs_jet4btag
inputs_jet4eta
inputs_jet4phi
inputs_jet4pt
inputs_leptoneta
inputs_leptonphi
inputs_leptonpt

inputs_mbb!
inputs_missingenergymagnitude
inputs_missingenergyphi

inputs_mjj
inputs_mjjj
inputs_mjlv

inputs_mlv
inputs_mwbb
inputs_mwwbb
inference_op_model_handle
identity¢inference_op	
PartitionedCallPartitionedCallinputs_jet1btaginputs_jet1etainputs_jet1phiinputs_jet1ptinputs_jet2btaginputs_jet2etainputs_jet2phiinputs_jet2ptinputs_jet3btaginputs_jet3etainputs_jet3phiinputs_jet3ptinputs_jet4btaginputs_jet4etainputs_jet4phiinputs_jet4ptinputs_leptonetainputs_leptonphiinputs_leptonpt
inputs_mbbinputs_missingenergymagnitudeinputs_missingenergyphi
inputs_mjjinputs_mjjjinputs_mjlv
inputs_mlvinputs_mwbbinputs_mwwbb*'
Tin 
2*(
Tout 
2*
_collective_manager_ids
 *º
_output_shapes§
¤:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ* 
_read_only_resource_inputs
 *-
config_proto

CPU

GPU 2J 8 *3
f.R,
*__inference__build_normalized_inputs_16133¾
stackPackPartitionedCall:output:0PartitionedCall:output:1PartitionedCall:output:2PartitionedCall:output:3PartitionedCall:output:4PartitionedCall:output:5PartitionedCall:output:6PartitionedCall:output:7PartitionedCall:output:8PartitionedCall:output:9PartitionedCall:output:10PartitionedCall:output:11PartitionedCall:output:12PartitionedCall:output:13PartitionedCall:output:14PartitionedCall:output:15PartitionedCall:output:16PartitionedCall:output:17PartitionedCall:output:18PartitionedCall:output:19PartitionedCall:output:20PartitionedCall:output:21PartitionedCall:output:22PartitionedCall:output:23PartitionedCall:output:24PartitionedCall:output:25PartitionedCall:output:26PartitionedCall:output:27*
N*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*

axisL
ConstConst*
_output_shapes
:  *
dtype0*
value
B  N
Const_1Const*
_output_shapes
:  *
dtype0*
value
B  X
RaggedConstant/valuesConst*
_output_shapes
: *
dtype0*
valueB ^
RaggedConstant/ConstConst*
_output_shapes
:*
dtype0	*
valueB	R `
RaggedConstant/Const_1Const*
_output_shapes
:*
dtype0	*
valueB	R ¡
inference_opSimpleMLInferenceOpWithHandlestack:output:0Const:output:0Const_1:output:0RaggedConstant/values:output:0RaggedConstant/Const:output:0RaggedConstant/Const_1:output:0inference_op_model_handle*-
_output_shapes
:ÿÿÿÿÿÿÿÿÿ:*
dense_output_dimd
strided_slice/stackConst*
_output_shapes
:*
dtype0*
valueB"       f
strided_slice/stack_1Const*
_output_shapes
:*
dtype0*
valueB"       f
strided_slice/stack_2Const*
_output_shapes
:*
dtype0*
valueB"      þ
strided_sliceStridedSlice inference_op:dense_predictions:0strided_slice/stack:output:0strided_slice/stack_1:output:0strided_slice/stack_2:output:0*
Index0*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*

begin_mask*
end_maske
IdentityIdentitystrided_slice:output:0^NoOp*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿU
NoOpNoOp^inference_op*"
_acd_function_control_output(*
_output_shapes
 "
identityIdentity:output:0*(
_construction_contextkEagerRuntime*»
_input_shapes©
¦:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ: 2
inference_opinference_op:T P
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet1btag:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet1eta:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet1phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet1pt:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet2btag:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet2eta:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet2phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet2pt:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet3btag:S	O
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet3eta:S
O
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet3phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet3pt:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet4btag:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet4eta:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet4phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet4pt:UQ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
*
_user_specified_nameinputs/leptoneta:UQ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
*
_user_specified_nameinputs/leptonphi:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/leptonpt:OK
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
$
_user_specified_name
inputs/mbb:b^
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
7
_user_specified_nameinputs/missingenergymagnitude:\X
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
1
_user_specified_nameinputs/missingenergyphi:OK
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
$
_user_specified_name
inputs/mjj:PL
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
%
_user_specified_nameinputs/mjjj:PL
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
%
_user_specified_nameinputs/mjlv:OK
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
$
_user_specified_name
inputs/mlv:PL
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
%
_user_specified_nameinputs/mwbb:QM
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
&
_user_specified_nameinputs/mwwbb
Ñ3
§
N__inference_random_forest_model_layer_call_and_return_conditional_losses_16424

inputs
inputs_1
inputs_2
inputs_3
inputs_4
inputs_5
inputs_6
inputs_7
inputs_8
inputs_9
	inputs_10
	inputs_11
	inputs_12
	inputs_13
	inputs_14
	inputs_15
	inputs_16
	inputs_17
	inputs_18
	inputs_19
	inputs_20
	inputs_21
	inputs_22
	inputs_23
	inputs_24
	inputs_25
	inputs_26
	inputs_27
inference_op_model_handle
identity¢inference_opÿ
PartitionedCallPartitionedCallinputsinputs_1inputs_2inputs_3inputs_4inputs_5inputs_6inputs_7inputs_8inputs_9	inputs_10	inputs_11	inputs_12	inputs_13	inputs_14	inputs_15	inputs_16	inputs_17	inputs_18	inputs_19	inputs_20	inputs_21	inputs_22	inputs_23	inputs_24	inputs_25	inputs_26	inputs_27*'
Tin 
2*(
Tout 
2*
_collective_manager_ids
 *º
_output_shapes§
¤:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ* 
_read_only_resource_inputs
 *-
config_proto

CPU

GPU 2J 8 *3
f.R,
*__inference__build_normalized_inputs_16133¾
stackPackPartitionedCall:output:0PartitionedCall:output:1PartitionedCall:output:2PartitionedCall:output:3PartitionedCall:output:4PartitionedCall:output:5PartitionedCall:output:6PartitionedCall:output:7PartitionedCall:output:8PartitionedCall:output:9PartitionedCall:output:10PartitionedCall:output:11PartitionedCall:output:12PartitionedCall:output:13PartitionedCall:output:14PartitionedCall:output:15PartitionedCall:output:16PartitionedCall:output:17PartitionedCall:output:18PartitionedCall:output:19PartitionedCall:output:20PartitionedCall:output:21PartitionedCall:output:22PartitionedCall:output:23PartitionedCall:output:24PartitionedCall:output:25PartitionedCall:output:26PartitionedCall:output:27*
N*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*

axisL
ConstConst*
_output_shapes
:  *
dtype0*
value
B  N
Const_1Const*
_output_shapes
:  *
dtype0*
value
B  X
RaggedConstant/valuesConst*
_output_shapes
: *
dtype0*
valueB ^
RaggedConstant/ConstConst*
_output_shapes
:*
dtype0	*
valueB	R `
RaggedConstant/Const_1Const*
_output_shapes
:*
dtype0	*
valueB	R ¡
inference_opSimpleMLInferenceOpWithHandlestack:output:0Const:output:0Const_1:output:0RaggedConstant/values:output:0RaggedConstant/Const:output:0RaggedConstant/Const_1:output:0inference_op_model_handle*-
_output_shapes
:ÿÿÿÿÿÿÿÿÿ:*
dense_output_dimd
strided_slice/stackConst*
_output_shapes
:*
dtype0*
valueB"       f
strided_slice/stack_1Const*
_output_shapes
:*
dtype0*
valueB"       f
strided_slice/stack_2Const*
_output_shapes
:*
dtype0*
valueB"      þ
strided_sliceStridedSlice inference_op:dense_predictions:0strided_slice/stack:output:0strided_slice/stack_1:output:0strided_slice/stack_2:output:0*
Index0*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*

begin_mask*
end_maske
IdentityIdentitystrided_slice:output:0^NoOp*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿU
NoOpNoOp^inference_op*"
_acd_function_control_output(*
_output_shapes
 "
identityIdentity:output:0*(
_construction_contextkEagerRuntime*»
_input_shapes©
¦:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ: 2
inference_opinference_op:K G
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:K	G
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:K
G
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs

,
__inference__destroyer_17009
identityG
ConstConst*
_output_shapes
: *
dtype0*
value	B :E
IdentityIdentityConst:output:0*
T0*
_output_shapes
: "
identityIdentity:output:0*(
_construction_contextkEagerRuntime*
_input_shapes 
ô
»
__inference_<lambda>_17017
staticregexreplace_input>
:simple_ml_simplemlloadmodelfrompathwithhandle_model_handle
identity¢-simple_ml/SimpleMLLoadModelFromPathWithHandle|
StaticRegexReplaceStaticRegexReplacestaticregexreplace_input*
_output_shapes
: *
patterndone*
rewrite Ã
-simple_ml/SimpleMLLoadModelFromPathWithHandle#SimpleMLLoadModelFromPathWithHandle:simple_ml_simplemlloadmodelfrompathwithhandle_model_handleStaticRegexReplace:output:0*
_output_shapes
 J
ConstConst*
_output_shapes
: *
dtype0*
valueB
 *  ?L
IdentityIdentityConst:output:0^NoOp*
T0*
_output_shapes
: v
NoOpNoOp.^simple_ml/SimpleMLLoadModelFromPathWithHandle*"
_acd_function_control_output(*
_output_shapes
 "
identityIdentity:output:0*(
_construction_contextkEagerRuntime*
_input_shapes
: : 2^
-simple_ml/SimpleMLLoadModelFromPathWithHandle-simple_ml/SimpleMLLoadModelFromPathWithHandle: 

_output_shapes
: 
µ$
£
3__inference_random_forest_model_layer_call_fn_16641
inputs_jet1btag
inputs_jet1eta
inputs_jet1phi
inputs_jet1pt
inputs_jet2btag
inputs_jet2eta
inputs_jet2phi
inputs_jet2pt
inputs_jet3btag
inputs_jet3eta
inputs_jet3phi
inputs_jet3pt
inputs_jet4btag
inputs_jet4eta
inputs_jet4phi
inputs_jet4pt
inputs_leptoneta
inputs_leptonphi
inputs_leptonpt

inputs_mbb!
inputs_missingenergymagnitude
inputs_missingenergyphi

inputs_mjj
inputs_mjjj
inputs_mjlv

inputs_mlv
inputs_mwbb
inputs_mwwbb
unknown
identity¢StatefulPartitionedCall¬
StatefulPartitionedCallStatefulPartitionedCallinputs_jet1btaginputs_jet1etainputs_jet1phiinputs_jet1ptinputs_jet2btaginputs_jet2etainputs_jet2phiinputs_jet2ptinputs_jet3btaginputs_jet3etainputs_jet3phiinputs_jet3ptinputs_jet4btaginputs_jet4etainputs_jet4phiinputs_jet4ptinputs_leptonetainputs_leptonphiinputs_leptonpt
inputs_mbbinputs_missingenergymagnitudeinputs_missingenergyphi
inputs_mjjinputs_mjjjinputs_mjlv
inputs_mlvinputs_mwbbinputs_mwwbbunknown*(
Tin!
2*
Tout
2*
_collective_manager_ids
 *'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ* 
_read_only_resource_inputs
 *-
config_proto

CPU

GPU 2J 8 *W
fRRP
N__inference_random_forest_model_layer_call_and_return_conditional_losses_16284o
IdentityIdentity StatefulPartitionedCall:output:0^NoOp*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ`
NoOpNoOp^StatefulPartitionedCall*"
_acd_function_control_output(*
_output_shapes
 "
identityIdentity:output:0*(
_construction_contextkEagerRuntime*»
_input_shapes©
¦:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ: 22
StatefulPartitionedCallStatefulPartitionedCall:T P
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet1btag:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet1eta:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet1phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet1pt:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet2btag:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet2eta:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet2phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet2pt:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet3btag:S	O
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet3eta:S
O
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet3phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet3pt:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet4btag:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet4eta:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet4phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet4pt:UQ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
*
_user_specified_nameinputs/leptoneta:UQ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
*
_user_specified_nameinputs/leptonphi:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/leptonpt:OK
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
$
_user_specified_name
inputs/mbb:b^
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
7
_user_specified_nameinputs/missingenergymagnitude:\X
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
1
_user_specified_nameinputs/missingenergyphi:OK
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
$
_user_specified_name
inputs/mjj:PL
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
%
_user_specified_nameinputs/mjjj:PL
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
%
_user_specified_nameinputs/mjlv:OK
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
$
_user_specified_name
inputs/mlv:PL
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
%
_user_specified_nameinputs/mwbb:QM
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
&
_user_specified_nameinputs/mwwbb
«

__inference__traced_save_17093
file_prefix)
%savev2_is_trained_read_readvariableop

savev2_const

identity_1¢MergeV2Checkpointsw
StaticRegexFullMatchStaticRegexFullMatchfile_prefix"/device:CPU:**
_output_shapes
: *
pattern
^s3://.*Z
ConstConst"/device:CPU:**
_output_shapes
: *
dtype0*
valueB B.parta
Const_1Const"/device:CPU:**
_output_shapes
: *
dtype0*
valueB B
_temp/part
SelectSelectStaticRegexFullMatch:output:0Const:output:0Const_1:output:0"/device:CPU:**
T0*
_output_shapes
: f

StringJoin
StringJoinfile_prefixSelect:output:0"/device:CPU:**
N*
_output_shapes
: L

num_shardsConst*
_output_shapes
: *
dtype0*
value	B :f
ShardedFilename/shardConst"/device:CPU:0*
_output_shapes
: *
dtype0*
value	B : 
ShardedFilenameShardedFilenameStringJoin:output:0ShardedFilename/shard:output:0num_shards:output:0"/device:CPU:0*
_output_shapes
: ¯
SaveV2/tensor_namesConst"/device:CPU:0*
_output_shapes
:*
dtype0*Y
valuePBNB&_is_trained/.ATTRIBUTES/VARIABLE_VALUEB_CHECKPOINTABLE_OBJECT_GRAPHq
SaveV2/shape_and_slicesConst"/device:CPU:0*
_output_shapes
:*
dtype0*
valueBB B Ø
SaveV2SaveV2ShardedFilename:filename:0SaveV2/tensor_names:output:0 SaveV2/shape_and_slices:output:0%savev2_is_trained_read_readvariableopsavev2_const"/device:CPU:0*
_output_shapes
 *
dtypes
2

&MergeV2Checkpoints/checkpoint_prefixesPackShardedFilename:filename:0^SaveV2"/device:CPU:0*
N*
T0*
_output_shapes
:
MergeV2CheckpointsMergeV2Checkpoints/MergeV2Checkpoints/checkpoint_prefixes:output:0file_prefix"/device:CPU:0*
_output_shapes
 f
IdentityIdentityfile_prefix^MergeV2Checkpoints"/device:CPU:0*
T0*
_output_shapes
: Q

Identity_1IdentityIdentity:output:0^NoOp*
T0*
_output_shapes
: [
NoOpNoOp^MergeV2Checkpoints*"
_acd_function_control_output(*
_output_shapes
 "!

identity_1Identity_1:output:0*
_input_shapes
: : : 2(
MergeV2CheckpointsMergeV2Checkpoints:C ?

_output_shapes
: 
%
_user_specified_namefile_prefix:

_output_shapes
: :

_output_shapes
: 
«
Ï
#__inference_signature_wrapper_16991
jet1btag
jet1eta
jet1phi

jet1pt
jet2btag
jet2eta
jet2phi

jet2pt
jet3btag
jet3eta
jet3phi

jet3pt
jet4btag
jet4eta
jet4phi

jet4pt
	leptoneta
	leptonphi
leptonpt
mbb
missingenergymagnitude
missingenergyphi
mjj
mjjj
mjlv
mlv
mwbb	
mwwbb
unknown
identity¢StatefulPartitionedCallº
StatefulPartitionedCallStatefulPartitionedCalljet1btagjet1etajet1phijet1ptjet2btagjet2etajet2phijet2ptjet3btagjet3etajet3phijet3ptjet4btagjet4etajet4phijet4pt	leptoneta	leptonphileptonptmbbmissingenergymagnitudemissingenergyphimjjmjjjmjlvmlvmwbbmwwbbunknown*(
Tin!
2*
Tout
2*
_collective_manager_ids
 *'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ* 
_read_only_resource_inputs
 *-
config_proto

CPU

GPU 2J 8 *)
f$R"
 __inference__wrapped_model_16181o
IdentityIdentity StatefulPartitionedCall:output:0^NoOp*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ`
NoOpNoOp^StatefulPartitionedCall*"
_acd_function_control_output(*
_output_shapes
 "
identityIdentity:output:0*(
_construction_contextkEagerRuntime*»
_input_shapes©
¦:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ: 22
StatefulPartitionedCallStatefulPartitionedCall:M I
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet1btag:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet1eta:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet1phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet1pt:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet2btag:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet2eta:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet2phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet2pt:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet3btag:L	H
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet3eta:L
H
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet3phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet3pt:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet4btag:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet4eta:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet4phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet4pt:NJ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
#
_user_specified_name	leptoneta:NJ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
#
_user_specified_name	leptonphi:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
leptonpt:HD
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namembb:[W
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
0
_user_specified_namemissingenergymagnitude:UQ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
*
_user_specified_namemissingenergyphi:HD
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemjj:IE
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemjjj:IE
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemjlv:HD
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemlv:IE
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemwbb:JF
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemwwbb
Ñ
K
__inference__creator_16996
identity¢SimpleMLCreateModelResource
SimpleMLCreateModelResourceSimpleMLCreateModelResource*
_output_shapes
: *E
shared_name64simple_ml_model_355c76c8-ee88-40e5-a8ab-d79fd5fd888ah
IdentityIdentity*SimpleMLCreateModelResource:model_handle:0^NoOp*
T0*
_output_shapes
: d
NoOpNoOp^SimpleMLCreateModelResource*"
_acd_function_control_output(*
_output_shapes
 "
identityIdentity:output:0*(
_construction_contextkEagerRuntime*
_input_shapes 2:
SimpleMLCreateModelResourceSimpleMLCreateModelResource
é
ß
3__inference_random_forest_model_layer_call_fn_16463
jet1btag
jet1eta
jet1phi

jet1pt
jet2btag
jet2eta
jet2phi

jet2pt
jet3btag
jet3eta
jet3phi

jet3pt
jet4btag
jet4eta
jet4phi

jet4pt
	leptoneta
	leptonphi
leptonpt
mbb
missingenergymagnitude
missingenergyphi
mjj
mjjj
mjlv
mlv
mwbb	
mwwbb
unknown
identity¢StatefulPartitionedCallè
StatefulPartitionedCallStatefulPartitionedCalljet1btagjet1etajet1phijet1ptjet2btagjet2etajet2phijet2ptjet3btagjet3etajet3phijet3ptjet4btagjet4etajet4phijet4pt	leptoneta	leptonphileptonptmbbmissingenergymagnitudemissingenergyphimjjmjjjmjlvmlvmwbbmwwbbunknown*(
Tin!
2*
Tout
2*
_collective_manager_ids
 *'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ* 
_read_only_resource_inputs
 *-
config_proto

CPU

GPU 2J 8 *W
fRRP
N__inference_random_forest_model_layer_call_and_return_conditional_losses_16424o
IdentityIdentity StatefulPartitionedCall:output:0^NoOp*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ`
NoOpNoOp^StatefulPartitionedCall*"
_acd_function_control_output(*
_output_shapes
 "
identityIdentity:output:0*(
_construction_contextkEagerRuntime*»
_input_shapes©
¦:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ: 22
StatefulPartitionedCallStatefulPartitionedCall:M I
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet1btag:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet1eta:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet1phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet1pt:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet2btag:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet2eta:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet2phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet2pt:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet3btag:L	H
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet3eta:L
H
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet3phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet3pt:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet4btag:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet4eta:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet4phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet4pt:NJ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
#
_user_specified_name	leptoneta:NJ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
#
_user_specified_name	leptonphi:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
leptonpt:HD
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namembb:[W
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
0
_user_specified_namemissingenergymagnitude:UQ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
*
_user_specified_namemissingenergyphi:HD
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemjj:IE
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemjjj:IE
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemjlv:HD
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemlv:IE
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemwbb:JF
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemwwbb
½


!__inference__traced_restore_17106
file_prefix%
assignvariableop_is_trained:
 

identity_2¢AssignVariableOp²
RestoreV2/tensor_namesConst"/device:CPU:0*
_output_shapes
:*
dtype0*Y
valuePBNB&_is_trained/.ATTRIBUTES/VARIABLE_VALUEB_CHECKPOINTABLE_OBJECT_GRAPHt
RestoreV2/shape_and_slicesConst"/device:CPU:0*
_output_shapes
:*
dtype0*
valueBB B ¨
	RestoreV2	RestoreV2file_prefixRestoreV2/tensor_names:output:0#RestoreV2/shape_and_slices:output:0"/device:CPU:0*
_output_shapes

::*
dtypes
2
[
IdentityIdentityRestoreV2:tensors:0"/device:CPU:0*
T0
*
_output_shapes
:
AssignVariableOpAssignVariableOpassignvariableop_is_trainedIdentity:output:0"/device:CPU:0*
_output_shapes
 *
dtype0
1
NoOpNoOp"/device:CPU:0*
_output_shapes
 m

Identity_1Identityfile_prefix^AssignVariableOp^NoOp"/device:CPU:0*
T0*
_output_shapes
: U

Identity_2IdentityIdentity_1:output:0^NoOp_1*
T0*
_output_shapes
: [
NoOp_1NoOp^AssignVariableOp*"
_acd_function_control_output(*
_output_shapes
 "!

identity_2Identity_2:output:0*
_input_shapes
: : 2$
AssignVariableOpAssignVariableOp:C ?

_output_shapes
: 
%
_user_specified_namefile_prefix
ó7
Å
N__inference_random_forest_model_layer_call_and_return_conditional_losses_16819
inputs_jet1btag
inputs_jet1eta
inputs_jet1phi
inputs_jet1pt
inputs_jet2btag
inputs_jet2eta
inputs_jet2phi
inputs_jet2pt
inputs_jet3btag
inputs_jet3eta
inputs_jet3phi
inputs_jet3pt
inputs_jet4btag
inputs_jet4eta
inputs_jet4phi
inputs_jet4pt
inputs_leptoneta
inputs_leptonphi
inputs_leptonpt

inputs_mbb!
inputs_missingenergymagnitude
inputs_missingenergyphi

inputs_mjj
inputs_mjjj
inputs_mjlv

inputs_mlv
inputs_mwbb
inputs_mwwbb
inference_op_model_handle
identity¢inference_op	
PartitionedCallPartitionedCallinputs_jet1btaginputs_jet1etainputs_jet1phiinputs_jet1ptinputs_jet2btaginputs_jet2etainputs_jet2phiinputs_jet2ptinputs_jet3btaginputs_jet3etainputs_jet3phiinputs_jet3ptinputs_jet4btaginputs_jet4etainputs_jet4phiinputs_jet4ptinputs_leptonetainputs_leptonphiinputs_leptonpt
inputs_mbbinputs_missingenergymagnitudeinputs_missingenergyphi
inputs_mjjinputs_mjjjinputs_mjlv
inputs_mlvinputs_mwbbinputs_mwwbb*'
Tin 
2*(
Tout 
2*
_collective_manager_ids
 *º
_output_shapes§
¤:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ* 
_read_only_resource_inputs
 *-
config_proto

CPU

GPU 2J 8 *3
f.R,
*__inference__build_normalized_inputs_16133¾
stackPackPartitionedCall:output:0PartitionedCall:output:1PartitionedCall:output:2PartitionedCall:output:3PartitionedCall:output:4PartitionedCall:output:5PartitionedCall:output:6PartitionedCall:output:7PartitionedCall:output:8PartitionedCall:output:9PartitionedCall:output:10PartitionedCall:output:11PartitionedCall:output:12PartitionedCall:output:13PartitionedCall:output:14PartitionedCall:output:15PartitionedCall:output:16PartitionedCall:output:17PartitionedCall:output:18PartitionedCall:output:19PartitionedCall:output:20PartitionedCall:output:21PartitionedCall:output:22PartitionedCall:output:23PartitionedCall:output:24PartitionedCall:output:25PartitionedCall:output:26PartitionedCall:output:27*
N*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*

axisL
ConstConst*
_output_shapes
:  *
dtype0*
value
B  N
Const_1Const*
_output_shapes
:  *
dtype0*
value
B  X
RaggedConstant/valuesConst*
_output_shapes
: *
dtype0*
valueB ^
RaggedConstant/ConstConst*
_output_shapes
:*
dtype0	*
valueB	R `
RaggedConstant/Const_1Const*
_output_shapes
:*
dtype0	*
valueB	R ¡
inference_opSimpleMLInferenceOpWithHandlestack:output:0Const:output:0Const_1:output:0RaggedConstant/values:output:0RaggedConstant/Const:output:0RaggedConstant/Const_1:output:0inference_op_model_handle*-
_output_shapes
:ÿÿÿÿÿÿÿÿÿ:*
dense_output_dimd
strided_slice/stackConst*
_output_shapes
:*
dtype0*
valueB"       f
strided_slice/stack_1Const*
_output_shapes
:*
dtype0*
valueB"       f
strided_slice/stack_2Const*
_output_shapes
:*
dtype0*
valueB"      þ
strided_sliceStridedSlice inference_op:dense_predictions:0strided_slice/stack:output:0strided_slice/stack_1:output:0strided_slice/stack_2:output:0*
Index0*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*

begin_mask*
end_maske
IdentityIdentitystrided_slice:output:0^NoOp*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿU
NoOpNoOp^inference_op*"
_acd_function_control_output(*
_output_shapes
 "
identityIdentity:output:0*(
_construction_contextkEagerRuntime*»
_input_shapes©
¦:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ: 2
inference_opinference_op:T P
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet1btag:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet1eta:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet1phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet1pt:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet2btag:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet2eta:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet2phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet2pt:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet3btag:S	O
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet3eta:S
O
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet3phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet3pt:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet4btag:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet4eta:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet4phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet4pt:UQ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
*
_user_specified_nameinputs/leptoneta:UQ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
*
_user_specified_nameinputs/leptonphi:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/leptonpt:OK
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
$
_user_specified_name
inputs/mbb:b^
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
7
_user_specified_nameinputs/missingenergymagnitude:\X
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
1
_user_specified_nameinputs/missingenergyphi:OK
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
$
_user_specified_name
inputs/mjj:PL
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
%
_user_specified_nameinputs/mjjj:PL
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
%
_user_specified_nameinputs/mjlv:OK
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
$
_user_specified_name
inputs/mlv:PL
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
%
_user_specified_nameinputs/mwbb:QM
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
&
_user_specified_nameinputs/mwwbb
­
[
-__inference_yggdrasil_model_path_tensor_16955
staticregexreplace_input
identity|
StaticRegexReplaceStaticRegexReplacestaticregexreplace_input*
_output_shapes
: *
patterndone*
rewrite R
IdentityIdentityStaticRegexReplace:output:0*
T0*
_output_shapes
: "
identityIdentity:output:0*(
_construction_contextkEagerRuntime*
_input_shapes
: : 

_output_shapes
: 
é
ß
3__inference_random_forest_model_layer_call_fn_16289
jet1btag
jet1eta
jet1phi

jet1pt
jet2btag
jet2eta
jet2phi

jet2pt
jet3btag
jet3eta
jet3phi

jet3pt
jet4btag
jet4eta
jet4phi

jet4pt
	leptoneta
	leptonphi
leptonpt
mbb
missingenergymagnitude
missingenergyphi
mjj
mjjj
mjlv
mlv
mwbb	
mwwbb
unknown
identity¢StatefulPartitionedCallè
StatefulPartitionedCallStatefulPartitionedCalljet1btagjet1etajet1phijet1ptjet2btagjet2etajet2phijet2ptjet3btagjet3etajet3phijet3ptjet4btagjet4etajet4phijet4pt	leptoneta	leptonphileptonptmbbmissingenergymagnitudemissingenergyphimjjmjjjmjlvmlvmwbbmwwbbunknown*(
Tin!
2*
Tout
2*
_collective_manager_ids
 *'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ* 
_read_only_resource_inputs
 *-
config_proto

CPU

GPU 2J 8 *W
fRRP
N__inference_random_forest_model_layer_call_and_return_conditional_losses_16284o
IdentityIdentity StatefulPartitionedCall:output:0^NoOp*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ`
NoOpNoOp^StatefulPartitionedCall*"
_acd_function_control_output(*
_output_shapes
 "
identityIdentity:output:0*(
_construction_contextkEagerRuntime*»
_input_shapes©
¦:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ: 22
StatefulPartitionedCallStatefulPartitionedCall:M I
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet1btag:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet1eta:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet1phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet1pt:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet2btag:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet2eta:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet2phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet2pt:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet3btag:L	H
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet3eta:L
H
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet3phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet3pt:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet4btag:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet4eta:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet4phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet4pt:NJ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
#
_user_specified_name	leptoneta:NJ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
#
_user_specified_name	leptonphi:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
leptonpt:HD
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namembb:[W
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
0
_user_specified_namemissingenergymagnitude:UQ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
*
_user_specified_namemissingenergyphi:HD
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemjj:IE
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemjjj:IE
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemjlv:HD
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemlv:IE
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemwbb:JF
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemwwbb
Ñ3
§
N__inference_random_forest_model_layer_call_and_return_conditional_losses_16284

inputs
inputs_1
inputs_2
inputs_3
inputs_4
inputs_5
inputs_6
inputs_7
inputs_8
inputs_9
	inputs_10
	inputs_11
	inputs_12
	inputs_13
	inputs_14
	inputs_15
	inputs_16
	inputs_17
	inputs_18
	inputs_19
	inputs_20
	inputs_21
	inputs_22
	inputs_23
	inputs_24
	inputs_25
	inputs_26
	inputs_27
inference_op_model_handle
identity¢inference_opÿ
PartitionedCallPartitionedCallinputsinputs_1inputs_2inputs_3inputs_4inputs_5inputs_6inputs_7inputs_8inputs_9	inputs_10	inputs_11	inputs_12	inputs_13	inputs_14	inputs_15	inputs_16	inputs_17	inputs_18	inputs_19	inputs_20	inputs_21	inputs_22	inputs_23	inputs_24	inputs_25	inputs_26	inputs_27*'
Tin 
2*(
Tout 
2*
_collective_manager_ids
 *º
_output_shapes§
¤:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ* 
_read_only_resource_inputs
 *-
config_proto

CPU

GPU 2J 8 *3
f.R,
*__inference__build_normalized_inputs_16133¾
stackPackPartitionedCall:output:0PartitionedCall:output:1PartitionedCall:output:2PartitionedCall:output:3PartitionedCall:output:4PartitionedCall:output:5PartitionedCall:output:6PartitionedCall:output:7PartitionedCall:output:8PartitionedCall:output:9PartitionedCall:output:10PartitionedCall:output:11PartitionedCall:output:12PartitionedCall:output:13PartitionedCall:output:14PartitionedCall:output:15PartitionedCall:output:16PartitionedCall:output:17PartitionedCall:output:18PartitionedCall:output:19PartitionedCall:output:20PartitionedCall:output:21PartitionedCall:output:22PartitionedCall:output:23PartitionedCall:output:24PartitionedCall:output:25PartitionedCall:output:26PartitionedCall:output:27*
N*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*

axisL
ConstConst*
_output_shapes
:  *
dtype0*
value
B  N
Const_1Const*
_output_shapes
:  *
dtype0*
value
B  X
RaggedConstant/valuesConst*
_output_shapes
: *
dtype0*
valueB ^
RaggedConstant/ConstConst*
_output_shapes
:*
dtype0	*
valueB	R `
RaggedConstant/Const_1Const*
_output_shapes
:*
dtype0	*
valueB	R ¡
inference_opSimpleMLInferenceOpWithHandlestack:output:0Const:output:0Const_1:output:0RaggedConstant/values:output:0RaggedConstant/Const:output:0RaggedConstant/Const_1:output:0inference_op_model_handle*-
_output_shapes
:ÿÿÿÿÿÿÿÿÿ:*
dense_output_dimd
strided_slice/stackConst*
_output_shapes
:*
dtype0*
valueB"       f
strided_slice/stack_1Const*
_output_shapes
:*
dtype0*
valueB"       f
strided_slice/stack_2Const*
_output_shapes
:*
dtype0*
valueB"      þ
strided_sliceStridedSlice inference_op:dense_predictions:0strided_slice/stack:output:0strided_slice/stack_1:output:0strided_slice/stack_2:output:0*
Index0*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*

begin_mask*
end_maske
IdentityIdentitystrided_slice:output:0^NoOp*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿU
NoOpNoOp^inference_op*"
_acd_function_control_output(*
_output_shapes
 "
identityIdentity:output:0*(
_construction_contextkEagerRuntime*»
_input_shapes©
¦:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ: 2
inference_opinference_op:K G
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:K	G
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:K
G
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs
3
ï
__inference_call_16176

inputs
inputs_1
inputs_2
inputs_3
inputs_4
inputs_5
inputs_6
inputs_7
inputs_8
inputs_9
	inputs_10
	inputs_11
	inputs_12
	inputs_13
	inputs_14
	inputs_15
	inputs_16
	inputs_17
	inputs_18
	inputs_19
	inputs_20
	inputs_21
	inputs_22
	inputs_23
	inputs_24
	inputs_25
	inputs_26
	inputs_27
inference_op_model_handle
identity¢inference_opÿ
PartitionedCallPartitionedCallinputsinputs_1inputs_2inputs_3inputs_4inputs_5inputs_6inputs_7inputs_8inputs_9	inputs_10	inputs_11	inputs_12	inputs_13	inputs_14	inputs_15	inputs_16	inputs_17	inputs_18	inputs_19	inputs_20	inputs_21	inputs_22	inputs_23	inputs_24	inputs_25	inputs_26	inputs_27*'
Tin 
2*(
Tout 
2*
_collective_manager_ids
 *º
_output_shapes§
¤:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ* 
_read_only_resource_inputs
 *-
config_proto

CPU

GPU 2J 8 *3
f.R,
*__inference__build_normalized_inputs_16133¾
stackPackPartitionedCall:output:0PartitionedCall:output:1PartitionedCall:output:2PartitionedCall:output:3PartitionedCall:output:4PartitionedCall:output:5PartitionedCall:output:6PartitionedCall:output:7PartitionedCall:output:8PartitionedCall:output:9PartitionedCall:output:10PartitionedCall:output:11PartitionedCall:output:12PartitionedCall:output:13PartitionedCall:output:14PartitionedCall:output:15PartitionedCall:output:16PartitionedCall:output:17PartitionedCall:output:18PartitionedCall:output:19PartitionedCall:output:20PartitionedCall:output:21PartitionedCall:output:22PartitionedCall:output:23PartitionedCall:output:24PartitionedCall:output:25PartitionedCall:output:26PartitionedCall:output:27*
N*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*

axisL
ConstConst*
_output_shapes
:  *
dtype0*
value
B  N
Const_1Const*
_output_shapes
:  *
dtype0*
value
B  X
RaggedConstant/valuesConst*
_output_shapes
: *
dtype0*
valueB ^
RaggedConstant/ConstConst*
_output_shapes
:*
dtype0	*
valueB	R `
RaggedConstant/Const_1Const*
_output_shapes
:*
dtype0	*
valueB	R ¡
inference_opSimpleMLInferenceOpWithHandlestack:output:0Const:output:0Const_1:output:0RaggedConstant/values:output:0RaggedConstant/Const:output:0RaggedConstant/Const_1:output:0inference_op_model_handle*-
_output_shapes
:ÿÿÿÿÿÿÿÿÿ:*
dense_output_dimd
strided_slice/stackConst*
_output_shapes
:*
dtype0*
valueB"       f
strided_slice/stack_1Const*
_output_shapes
:*
dtype0*
valueB"       f
strided_slice/stack_2Const*
_output_shapes
:*
dtype0*
valueB"      þ
strided_sliceStridedSlice inference_op:dense_predictions:0strided_slice/stack:output:0strided_slice/stack_1:output:0strided_slice/stack_2:output:0*
Index0*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*

begin_mask*
end_maske
IdentityIdentitystrided_slice:output:0^NoOp*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿU
NoOpNoOp^inference_op*"
_acd_function_control_output(*
_output_shapes
 "
identityIdentity:output:0*(
_construction_contextkEagerRuntime*»
_input_shapes©
¦:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ: 2
inference_opinference_op:K G
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:K	G
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:K
G
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs
µ$
£
3__inference_random_forest_model_layer_call_fn_16675
inputs_jet1btag
inputs_jet1eta
inputs_jet1phi
inputs_jet1pt
inputs_jet2btag
inputs_jet2eta
inputs_jet2phi
inputs_jet2pt
inputs_jet3btag
inputs_jet3eta
inputs_jet3phi
inputs_jet3pt
inputs_jet4btag
inputs_jet4eta
inputs_jet4phi
inputs_jet4pt
inputs_leptoneta
inputs_leptonphi
inputs_leptonpt

inputs_mbb!
inputs_missingenergymagnitude
inputs_missingenergyphi

inputs_mjj
inputs_mjjj
inputs_mjlv

inputs_mlv
inputs_mwbb
inputs_mwwbb
unknown
identity¢StatefulPartitionedCall¬
StatefulPartitionedCallStatefulPartitionedCallinputs_jet1btaginputs_jet1etainputs_jet1phiinputs_jet1ptinputs_jet2btaginputs_jet2etainputs_jet2phiinputs_jet2ptinputs_jet3btaginputs_jet3etainputs_jet3phiinputs_jet3ptinputs_jet4btaginputs_jet4etainputs_jet4phiinputs_jet4ptinputs_leptonetainputs_leptonphiinputs_leptonpt
inputs_mbbinputs_missingenergymagnitudeinputs_missingenergyphi
inputs_mjjinputs_mjjjinputs_mjlv
inputs_mlvinputs_mwbbinputs_mwwbbunknown*(
Tin!
2*
Tout
2*
_collective_manager_ids
 *'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ* 
_read_only_resource_inputs
 *-
config_proto

CPU

GPU 2J 8 *W
fRRP
N__inference_random_forest_model_layer_call_and_return_conditional_losses_16424o
IdentityIdentity StatefulPartitionedCall:output:0^NoOp*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ`
NoOpNoOp^StatefulPartitionedCall*"
_acd_function_control_output(*
_output_shapes
 "
identityIdentity:output:0*(
_construction_contextkEagerRuntime*»
_input_shapes©
¦:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ: 22
StatefulPartitionedCallStatefulPartitionedCall:T P
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet1btag:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet1eta:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet1phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet1pt:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet2btag:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet2eta:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet2phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet2pt:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet3btag:S	O
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet3eta:S
O
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet3phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet3pt:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet4btag:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet4eta:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet4phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet4pt:UQ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
*
_user_specified_nameinputs/leptoneta:UQ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
*
_user_specified_nameinputs/leptonphi:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/leptonpt:OK
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
$
_user_specified_name
inputs/mbb:b^
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
7
_user_specified_nameinputs/missingenergymagnitude:\X
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
1
_user_specified_nameinputs/missingenergyphi:OK
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
$
_user_specified_name
inputs/mjj:PL
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
%
_user_specified_nameinputs/mjjj:PL
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
%
_user_specified_nameinputs/mjlv:OK
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
$
_user_specified_name
inputs/mlv:PL
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
%
_user_specified_nameinputs/mwbb:QM
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
&
_user_specified_nameinputs/mwwbb
» 
ò
 __inference__wrapped_model_16181
jet1btag
jet1eta
jet1phi

jet1pt
jet2btag
jet2eta
jet2phi

jet2pt
jet3btag
jet3eta
jet3phi

jet3pt
jet4btag
jet4eta
jet4phi

jet4pt
	leptoneta
	leptonphi
leptonpt
mbb
missingenergymagnitude
missingenergyphi
mjj
mjjj
mjlv
mlv
mwbb	
mwwbb
random_forest_model_16177
identity¢+random_forest_model/StatefulPartitionedCallÖ
+random_forest_model/StatefulPartitionedCallStatefulPartitionedCalljet1btagjet1etajet1phijet1ptjet2btagjet2etajet2phijet2ptjet3btagjet3etajet3phijet3ptjet4btagjet4etajet4phijet4pt	leptoneta	leptonphileptonptmbbmissingenergymagnitudemissingenergyphimjjmjjjmjlvmlvmwbbmwwbbrandom_forest_model_16177*(
Tin!
2*
Tout
2*
_collective_manager_ids
 *'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ* 
_read_only_resource_inputs
 *-
config_proto

CPU

GPU 2J 8 *
fR
__inference_call_16176
IdentityIdentity4random_forest_model/StatefulPartitionedCall:output:0^NoOp*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿt
NoOpNoOp,^random_forest_model/StatefulPartitionedCall*"
_acd_function_control_output(*
_output_shapes
 "
identityIdentity:output:0*(
_construction_contextkEagerRuntime*»
_input_shapes©
¦:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ: 2Z
+random_forest_model/StatefulPartitionedCall+random_forest_model/StatefulPartitionedCall:M I
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet1btag:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet1eta:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet1phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet1pt:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet2btag:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet2eta:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet2phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet2pt:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet3btag:L	H
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet3eta:L
H
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet3phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet3pt:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet4btag:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet4eta:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet4phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet4pt:NJ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
#
_user_specified_name	leptoneta:NJ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
#
_user_specified_name	leptonphi:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
leptonpt:HD
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namembb:[W
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
0
_user_specified_namemissingenergymagnitude:UQ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
*
_user_specified_namemissingenergyphi:HD
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemjj:IE
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemjjj:IE
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemjlv:HD
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemlv:IE
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemwbb:JF
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemwwbb
½5

*__inference__build_normalized_inputs_16133

inputs
inputs_1
inputs_2
inputs_3
inputs_4
inputs_5
inputs_6
inputs_7
inputs_8
inputs_9
	inputs_10
	inputs_11
	inputs_12
	inputs_13
	inputs_14
	inputs_15
	inputs_16
	inputs_17
	inputs_18
	inputs_19
	inputs_20
	inputs_21
	inputs_22
	inputs_23
	inputs_24
	inputs_25
	inputs_26
	inputs_27
identity

identity_1

identity_2

identity_3

identity_4

identity_5

identity_6

identity_7

identity_8

identity_9
identity_10
identity_11
identity_12
identity_13
identity_14
identity_15
identity_16
identity_17
identity_18
identity_19
identity_20
identity_21
identity_22
identity_23
identity_24
identity_25
identity_26
identity_27J
IdentityIdentityinputs*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿN

Identity_1Identityinputs_1*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿN

Identity_2Identityinputs_2*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿN

Identity_3Identityinputs_3*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿN

Identity_4Identityinputs_4*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿN

Identity_5Identityinputs_5*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿN

Identity_6Identityinputs_6*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿN

Identity_7Identityinputs_7*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿN

Identity_8Identityinputs_8*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿN

Identity_9Identityinputs_9*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿP
Identity_10Identity	inputs_10*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿP
Identity_11Identity	inputs_11*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿP
Identity_12Identity	inputs_12*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿP
Identity_13Identity	inputs_13*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿP
Identity_14Identity	inputs_14*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿP
Identity_15Identity	inputs_15*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿP
Identity_16Identity	inputs_16*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿP
Identity_17Identity	inputs_17*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿP
Identity_18Identity	inputs_18*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿP
Identity_19Identity	inputs_19*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿP
Identity_20Identity	inputs_20*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿP
Identity_21Identity	inputs_21*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿP
Identity_22Identity	inputs_22*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿP
Identity_23Identity	inputs_23*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿP
Identity_24Identity	inputs_24*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿP
Identity_25Identity	inputs_25*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿP
Identity_26Identity	inputs_26*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿP
Identity_27Identity	inputs_27*
T0*#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ"
identityIdentity:output:0"!

identity_1Identity_1:output:0"#
identity_10Identity_10:output:0"#
identity_11Identity_11:output:0"#
identity_12Identity_12:output:0"#
identity_13Identity_13:output:0"#
identity_14Identity_14:output:0"#
identity_15Identity_15:output:0"#
identity_16Identity_16:output:0"#
identity_17Identity_17:output:0"#
identity_18Identity_18:output:0"#
identity_19Identity_19:output:0"!

identity_2Identity_2:output:0"#
identity_20Identity_20:output:0"#
identity_21Identity_21:output:0"#
identity_22Identity_22:output:0"#
identity_23Identity_23:output:0"#
identity_24Identity_24:output:0"#
identity_25Identity_25:output:0"#
identity_26Identity_26:output:0"#
identity_27Identity_27:output:0"!

identity_3Identity_3:output:0"!

identity_4Identity_4:output:0"!

identity_5Identity_5:output:0"!

identity_6Identity_6:output:0"!

identity_7Identity_7:output:0"!

identity_8Identity_8:output:0"!

identity_9Identity_9:output:0*(
_construction_contextkEagerRuntime*¹
_input_shapes§
¤:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:K G
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:K	G
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:K
G
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_nameinputs
õ
¿
__inference__initializer_17004
staticregexreplace_input>
:simple_ml_simplemlloadmodelfrompathwithhandle_model_handle
identity¢-simple_ml/SimpleMLLoadModelFromPathWithHandle|
StaticRegexReplaceStaticRegexReplacestaticregexreplace_input*
_output_shapes
: *
patterndone*
rewrite Ã
-simple_ml/SimpleMLLoadModelFromPathWithHandle#SimpleMLLoadModelFromPathWithHandle:simple_ml_simplemlloadmodelfrompathwithhandle_model_handleStaticRegexReplace:output:0*
_output_shapes
 G
ConstConst*
_output_shapes
: *
dtype0*
value	B :L
IdentityIdentityConst:output:0^NoOp*
T0*
_output_shapes
: v
NoOpNoOp.^simple_ml/SimpleMLLoadModelFromPathWithHandle*"
_acd_function_control_output(*
_output_shapes
 "
identityIdentity:output:0*(
_construction_contextkEagerRuntime*
_input_shapes
: : 2^
-simple_ml/SimpleMLLoadModelFromPathWithHandle-simple_ml/SimpleMLLoadModelFromPathWithHandle: 

_output_shapes
: 
§3

N__inference_random_forest_model_layer_call_and_return_conditional_losses_16535
jet1btag
jet1eta
jet1phi

jet1pt
jet2btag
jet2eta
jet2phi

jet2pt
jet3btag
jet3eta
jet3phi

jet3pt
jet4btag
jet4eta
jet4phi

jet4pt
	leptoneta
	leptonphi
leptonpt
mbb
missingenergymagnitude
missingenergyphi
mjj
mjjj
mjlv
mlv
mwbb	
mwwbb
inference_op_model_handle
identity¢inference_opÙ
PartitionedCallPartitionedCalljet1btagjet1etajet1phijet1ptjet2btagjet2etajet2phijet2ptjet3btagjet3etajet3phijet3ptjet4btagjet4etajet4phijet4pt	leptoneta	leptonphileptonptmbbmissingenergymagnitudemissingenergyphimjjmjjjmjlvmlvmwbbmwwbb*'
Tin 
2*(
Tout 
2*
_collective_manager_ids
 *º
_output_shapes§
¤:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ* 
_read_only_resource_inputs
 *-
config_proto

CPU

GPU 2J 8 *3
f.R,
*__inference__build_normalized_inputs_16133¾
stackPackPartitionedCall:output:0PartitionedCall:output:1PartitionedCall:output:2PartitionedCall:output:3PartitionedCall:output:4PartitionedCall:output:5PartitionedCall:output:6PartitionedCall:output:7PartitionedCall:output:8PartitionedCall:output:9PartitionedCall:output:10PartitionedCall:output:11PartitionedCall:output:12PartitionedCall:output:13PartitionedCall:output:14PartitionedCall:output:15PartitionedCall:output:16PartitionedCall:output:17PartitionedCall:output:18PartitionedCall:output:19PartitionedCall:output:20PartitionedCall:output:21PartitionedCall:output:22PartitionedCall:output:23PartitionedCall:output:24PartitionedCall:output:25PartitionedCall:output:26PartitionedCall:output:27*
N*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*

axisL
ConstConst*
_output_shapes
:  *
dtype0*
value
B  N
Const_1Const*
_output_shapes
:  *
dtype0*
value
B  X
RaggedConstant/valuesConst*
_output_shapes
: *
dtype0*
valueB ^
RaggedConstant/ConstConst*
_output_shapes
:*
dtype0	*
valueB	R `
RaggedConstant/Const_1Const*
_output_shapes
:*
dtype0	*
valueB	R ¡
inference_opSimpleMLInferenceOpWithHandlestack:output:0Const:output:0Const_1:output:0RaggedConstant/values:output:0RaggedConstant/Const:output:0RaggedConstant/Const_1:output:0inference_op_model_handle*-
_output_shapes
:ÿÿÿÿÿÿÿÿÿ:*
dense_output_dimd
strided_slice/stackConst*
_output_shapes
:*
dtype0*
valueB"       f
strided_slice/stack_1Const*
_output_shapes
:*
dtype0*
valueB"       f
strided_slice/stack_2Const*
_output_shapes
:*
dtype0*
valueB"      þ
strided_sliceStridedSlice inference_op:dense_predictions:0strided_slice/stack:output:0strided_slice/stack_1:output:0strided_slice/stack_2:output:0*
Index0*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*

begin_mask*
end_maske
IdentityIdentitystrided_slice:output:0^NoOp*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿU
NoOpNoOp^inference_op*"
_acd_function_control_output(*
_output_shapes
 "
identityIdentity:output:0*(
_construction_contextkEagerRuntime*»
_input_shapes©
¦:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ: 2
inference_opinference_op:M I
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet1btag:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet1eta:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet1phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet1pt:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet2btag:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet2eta:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet2phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet2pt:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet3btag:L	H
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet3eta:L
H
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet3phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet3pt:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet4btag:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet4eta:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet4phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet4pt:NJ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
#
_user_specified_name	leptoneta:NJ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
#
_user_specified_name	leptonphi:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
leptonpt:HD
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namembb:[W
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
0
_user_specified_namemissingenergymagnitude:UQ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
*
_user_specified_namemissingenergyphi:HD
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemjj:IE
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemjjj:IE
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemjlv:HD
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemlv:IE
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemwbb:JF
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemwwbb
§3

N__inference_random_forest_model_layer_call_and_return_conditional_losses_16607
jet1btag
jet1eta
jet1phi

jet1pt
jet2btag
jet2eta
jet2phi

jet2pt
jet3btag
jet3eta
jet3phi

jet3pt
jet4btag
jet4eta
jet4phi

jet4pt
	leptoneta
	leptonphi
leptonpt
mbb
missingenergymagnitude
missingenergyphi
mjj
mjjj
mjlv
mlv
mwbb	
mwwbb
inference_op_model_handle
identity¢inference_opÙ
PartitionedCallPartitionedCalljet1btagjet1etajet1phijet1ptjet2btagjet2etajet2phijet2ptjet3btagjet3etajet3phijet3ptjet4btagjet4etajet4phijet4pt	leptoneta	leptonphileptonptmbbmissingenergymagnitudemissingenergyphimjjmjjjmjlvmlvmwbbmwwbb*'
Tin 
2*(
Tout 
2*
_collective_manager_ids
 *º
_output_shapes§
¤:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ* 
_read_only_resource_inputs
 *-
config_proto

CPU

GPU 2J 8 *3
f.R,
*__inference__build_normalized_inputs_16133¾
stackPackPartitionedCall:output:0PartitionedCall:output:1PartitionedCall:output:2PartitionedCall:output:3PartitionedCall:output:4PartitionedCall:output:5PartitionedCall:output:6PartitionedCall:output:7PartitionedCall:output:8PartitionedCall:output:9PartitionedCall:output:10PartitionedCall:output:11PartitionedCall:output:12PartitionedCall:output:13PartitionedCall:output:14PartitionedCall:output:15PartitionedCall:output:16PartitionedCall:output:17PartitionedCall:output:18PartitionedCall:output:19PartitionedCall:output:20PartitionedCall:output:21PartitionedCall:output:22PartitionedCall:output:23PartitionedCall:output:24PartitionedCall:output:25PartitionedCall:output:26PartitionedCall:output:27*
N*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*

axisL
ConstConst*
_output_shapes
:  *
dtype0*
value
B  N
Const_1Const*
_output_shapes
:  *
dtype0*
value
B  X
RaggedConstant/valuesConst*
_output_shapes
: *
dtype0*
valueB ^
RaggedConstant/ConstConst*
_output_shapes
:*
dtype0	*
valueB	R `
RaggedConstant/Const_1Const*
_output_shapes
:*
dtype0	*
valueB	R ¡
inference_opSimpleMLInferenceOpWithHandlestack:output:0Const:output:0Const_1:output:0RaggedConstant/values:output:0RaggedConstant/Const:output:0RaggedConstant/Const_1:output:0inference_op_model_handle*-
_output_shapes
:ÿÿÿÿÿÿÿÿÿ:*
dense_output_dimd
strided_slice/stackConst*
_output_shapes
:*
dtype0*
valueB"       f
strided_slice/stack_1Const*
_output_shapes
:*
dtype0*
valueB"       f
strided_slice/stack_2Const*
_output_shapes
:*
dtype0*
valueB"      þ
strided_sliceStridedSlice inference_op:dense_predictions:0strided_slice/stack:output:0strided_slice/stack_1:output:0strided_slice/stack_2:output:0*
Index0*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*

begin_mask*
end_maske
IdentityIdentitystrided_slice:output:0^NoOp*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿU
NoOpNoOp^inference_op*"
_acd_function_control_output(*
_output_shapes
 "
identityIdentity:output:0*(
_construction_contextkEagerRuntime*»
_input_shapes©
¦:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ: 2
inference_opinference_op:M I
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet1btag:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet1eta:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet1phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet1pt:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet2btag:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet2eta:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet2phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet2pt:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet3btag:L	H
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet3eta:L
H
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet3phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet3pt:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
jet4btag:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet4eta:LH
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
!
_user_specified_name	jet4phi:KG
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
 
_user_specified_namejet4pt:NJ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
#
_user_specified_name	leptoneta:NJ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
#
_user_specified_name	leptonphi:MI
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
"
_user_specified_name
leptonpt:HD
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namembb:[W
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
0
_user_specified_namemissingenergymagnitude:UQ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
*
_user_specified_namemissingenergyphi:HD
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemjj:IE
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemjjj:IE
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemjlv:HD
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemlv:IE
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemwbb:JF
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ

_user_specified_namemwwbb
ó7
Å
N__inference_random_forest_model_layer_call_and_return_conditional_losses_16747
inputs_jet1btag
inputs_jet1eta
inputs_jet1phi
inputs_jet1pt
inputs_jet2btag
inputs_jet2eta
inputs_jet2phi
inputs_jet2pt
inputs_jet3btag
inputs_jet3eta
inputs_jet3phi
inputs_jet3pt
inputs_jet4btag
inputs_jet4eta
inputs_jet4phi
inputs_jet4pt
inputs_leptoneta
inputs_leptonphi
inputs_leptonpt

inputs_mbb!
inputs_missingenergymagnitude
inputs_missingenergyphi

inputs_mjj
inputs_mjjj
inputs_mjlv

inputs_mlv
inputs_mwbb
inputs_mwwbb
inference_op_model_handle
identity¢inference_op	
PartitionedCallPartitionedCallinputs_jet1btaginputs_jet1etainputs_jet1phiinputs_jet1ptinputs_jet2btaginputs_jet2etainputs_jet2phiinputs_jet2ptinputs_jet3btaginputs_jet3etainputs_jet3phiinputs_jet3ptinputs_jet4btaginputs_jet4etainputs_jet4phiinputs_jet4ptinputs_leptonetainputs_leptonphiinputs_leptonpt
inputs_mbbinputs_missingenergymagnitudeinputs_missingenergyphi
inputs_mjjinputs_mjjjinputs_mjlv
inputs_mlvinputs_mwbbinputs_mwwbb*'
Tin 
2*(
Tout 
2*
_collective_manager_ids
 *º
_output_shapes§
¤:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ* 
_read_only_resource_inputs
 *-
config_proto

CPU

GPU 2J 8 *3
f.R,
*__inference__build_normalized_inputs_16133¾
stackPackPartitionedCall:output:0PartitionedCall:output:1PartitionedCall:output:2PartitionedCall:output:3PartitionedCall:output:4PartitionedCall:output:5PartitionedCall:output:6PartitionedCall:output:7PartitionedCall:output:8PartitionedCall:output:9PartitionedCall:output:10PartitionedCall:output:11PartitionedCall:output:12PartitionedCall:output:13PartitionedCall:output:14PartitionedCall:output:15PartitionedCall:output:16PartitionedCall:output:17PartitionedCall:output:18PartitionedCall:output:19PartitionedCall:output:20PartitionedCall:output:21PartitionedCall:output:22PartitionedCall:output:23PartitionedCall:output:24PartitionedCall:output:25PartitionedCall:output:26PartitionedCall:output:27*
N*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*

axisL
ConstConst*
_output_shapes
:  *
dtype0*
value
B  N
Const_1Const*
_output_shapes
:  *
dtype0*
value
B  X
RaggedConstant/valuesConst*
_output_shapes
: *
dtype0*
valueB ^
RaggedConstant/ConstConst*
_output_shapes
:*
dtype0	*
valueB	R `
RaggedConstant/Const_1Const*
_output_shapes
:*
dtype0	*
valueB	R ¡
inference_opSimpleMLInferenceOpWithHandlestack:output:0Const:output:0Const_1:output:0RaggedConstant/values:output:0RaggedConstant/Const:output:0RaggedConstant/Const_1:output:0inference_op_model_handle*-
_output_shapes
:ÿÿÿÿÿÿÿÿÿ:*
dense_output_dimd
strided_slice/stackConst*
_output_shapes
:*
dtype0*
valueB"       f
strided_slice/stack_1Const*
_output_shapes
:*
dtype0*
valueB"       f
strided_slice/stack_2Const*
_output_shapes
:*
dtype0*
valueB"      þ
strided_sliceStridedSlice inference_op:dense_predictions:0strided_slice/stack:output:0strided_slice/stack_1:output:0strided_slice/stack_2:output:0*
Index0*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿ*

begin_mask*
end_maske
IdentityIdentitystrided_slice:output:0^NoOp*
T0*'
_output_shapes
:ÿÿÿÿÿÿÿÿÿU
NoOpNoOp^inference_op*"
_acd_function_control_output(*
_output_shapes
 "
identityIdentity:output:0*(
_construction_contextkEagerRuntime*»
_input_shapes©
¦:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ:ÿÿÿÿÿÿÿÿÿ: 2
inference_opinference_op:T P
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet1btag:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet1eta:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet1phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet1pt:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet2btag:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet2eta:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet2phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet2pt:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet3btag:S	O
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet3eta:S
O
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet3phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet3pt:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/jet4btag:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet4eta:SO
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
(
_user_specified_nameinputs/jet4phi:RN
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
'
_user_specified_nameinputs/jet4pt:UQ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
*
_user_specified_nameinputs/leptoneta:UQ
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
*
_user_specified_nameinputs/leptonphi:TP
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
)
_user_specified_nameinputs/leptonpt:OK
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
$
_user_specified_name
inputs/mbb:b^
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
7
_user_specified_nameinputs/missingenergymagnitude:\X
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
1
_user_specified_nameinputs/missingenergyphi:OK
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
$
_user_specified_name
inputs/mjj:PL
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
%
_user_specified_nameinputs/mjjj:PL
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
%
_user_specified_nameinputs/mjlv:OK
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
$
_user_specified_name
inputs/mlv:PL
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
%
_user_specified_nameinputs/mwbb:QM
#
_output_shapes
:ÿÿÿÿÿÿÿÿÿ
&
_user_specified_nameinputs/mwwbb"ÛL
saver_filename:0StatefulPartitionedCall_2:0StatefulPartitionedCall_38"
saved_model_main_op

NoOp*>
__saved_model_init_op%#
__saved_model_init_op

NoOp*¸
serving_default¤
9
jet1btag-
serving_default_jet1btag:0ÿÿÿÿÿÿÿÿÿ
7
jet1eta,
serving_default_jet1eta:0ÿÿÿÿÿÿÿÿÿ
7
jet1phi,
serving_default_jet1phi:0ÿÿÿÿÿÿÿÿÿ
5
jet1pt+
serving_default_jet1pt:0ÿÿÿÿÿÿÿÿÿ
9
jet2btag-
serving_default_jet2btag:0ÿÿÿÿÿÿÿÿÿ
7
jet2eta,
serving_default_jet2eta:0ÿÿÿÿÿÿÿÿÿ
7
jet2phi,
serving_default_jet2phi:0ÿÿÿÿÿÿÿÿÿ
5
jet2pt+
serving_default_jet2pt:0ÿÿÿÿÿÿÿÿÿ
9
jet3btag-
serving_default_jet3btag:0ÿÿÿÿÿÿÿÿÿ
7
jet3eta,
serving_default_jet3eta:0ÿÿÿÿÿÿÿÿÿ
7
jet3phi,
serving_default_jet3phi:0ÿÿÿÿÿÿÿÿÿ
5
jet3pt+
serving_default_jet3pt:0ÿÿÿÿÿÿÿÿÿ
9
jet4btag-
serving_default_jet4btag:0ÿÿÿÿÿÿÿÿÿ
7
jet4eta,
serving_default_jet4eta:0ÿÿÿÿÿÿÿÿÿ
7
jet4phi,
serving_default_jet4phi:0ÿÿÿÿÿÿÿÿÿ
5
jet4pt+
serving_default_jet4pt:0ÿÿÿÿÿÿÿÿÿ
;
	leptoneta.
serving_default_leptoneta:0ÿÿÿÿÿÿÿÿÿ
;
	leptonphi.
serving_default_leptonphi:0ÿÿÿÿÿÿÿÿÿ
9
leptonpt-
serving_default_leptonpt:0ÿÿÿÿÿÿÿÿÿ
/
mbb(
serving_default_mbb:0ÿÿÿÿÿÿÿÿÿ
U
missingenergymagnitude;
(serving_default_missingenergymagnitude:0ÿÿÿÿÿÿÿÿÿ
I
missingenergyphi5
"serving_default_missingenergyphi:0ÿÿÿÿÿÿÿÿÿ
/
mjj(
serving_default_mjj:0ÿÿÿÿÿÿÿÿÿ
1
mjjj)
serving_default_mjjj:0ÿÿÿÿÿÿÿÿÿ
1
mjlv)
serving_default_mjlv:0ÿÿÿÿÿÿÿÿÿ
/
mlv(
serving_default_mlv:0ÿÿÿÿÿÿÿÿÿ
1
mwbb)
serving_default_mwbb:0ÿÿÿÿÿÿÿÿÿ
3
mwwbb*
serving_default_mwwbb:0ÿÿÿÿÿÿÿÿÿ>
output_12
StatefulPartitionedCall_1:0ÿÿÿÿÿÿÿÿÿtensorflow/serving/predict2"

asset_path_initializer:0done27

asset_path_initializer_1:0random_forest_header.pb2,

asset_path_initializer_2:0data_spec.pb24

asset_path_initializer_3:0nodes-00000-of-000012)

asset_path_initializer_4:0	header.pb:·¶

_learner_params
	_features
_is_trained
	optimizer
loss

_model
	variables
trainable_variables
	regularization_losses

	keras_api
__call__
*&call_and_return_all_conditional_losses
_default_save_signature
_build_normalized_inputs
call
call_get_leaves
yggdrasil_model_path_tensor

signatures"
_tf_keras_model
 "
trackable_dict_wrapper
 "
trackable_list_wrapper
:
 2
is_trained
"
	optimizer
 "
trackable_dict_wrapper
G
_input_builder
_compiled_model"
_generic_user_object
'
0"
trackable_list_wrapper
 "
trackable_list_wrapper
 "
trackable_list_wrapper
Ê
non_trainable_variables

layers
metrics
layer_regularization_losses
layer_metrics
	variables
trainable_variables
	regularization_losses
__call__
_default_save_signature
*&call_and_return_all_conditional_losses
&"call_and_return_conditional_losses"
_generic_user_object
2
3__inference_random_forest_model_layer_call_fn_16289
3__inference_random_forest_model_layer_call_fn_16641
3__inference_random_forest_model_layer_call_fn_16675
3__inference_random_forest_model_layer_call_fn_16463´
«²§
FullArgSpec)
args!
jself
jinputs

jtraining
varargs
 
varkw
 
defaults
p 

kwonlyargs 
kwonlydefaultsª 
annotationsª *
 
ú2÷
N__inference_random_forest_model_layer_call_and_return_conditional_losses_16747
N__inference_random_forest_model_layer_call_and_return_conditional_losses_16819
N__inference_random_forest_model_layer_call_and_return_conditional_losses_16535
N__inference_random_forest_model_layer_call_and_return_conditional_losses_16607´
«²§
FullArgSpec)
args!
jself
jinputs

jtraining
varargs
 
varkw
 
defaults
p 

kwonlyargs 
kwonlydefaultsª 
annotationsª *
 
ÄBÁ
 __inference__wrapped_model_16181jet1btagjet1etajet1phijet1ptjet2btagjet2etajet2phijet2ptjet3btagjet3etajet3phijet3ptjet4btagjet4etajet4phijet4pt	leptoneta	leptonphileptonptmbbmissingenergymagnitudemissingenergyphimjjmjjjmjlvmlvmwbbmwwbb"
²
FullArgSpec
args 
varargsjargs
varkwjkwargs
defaults
 

kwonlyargs 
kwonlydefaults
 
annotationsª *
 
Ô2Ñ
*__inference__build_normalized_inputs_16877¢
²
FullArgSpec
args
jself
jinputs
varargs
 
varkw
 
defaults
 

kwonlyargs 
kwonlydefaults
 
annotationsª *
 
Ñ2Î
__inference_call_16949³
ª²¦
FullArgSpec)
args!
jself
jinputs

jtraining
varargs
 
varkw
 
defaults¢
p 

kwonlyargs 
kwonlydefaults
 
annotationsª *
 
¨2¥¢
²
FullArgSpec
args
jself
jinputs
varargs
 
varkw
 
defaults
 

kwonlyargs 
kwonlydefaults
 
annotationsª *
 
Î2Ë
-__inference_yggdrasil_model_path_tensor_16955
²
FullArgSpec
args
jself
varargs
 
varkw
 
defaults
 

kwonlyargs 
kwonlydefaults
 
annotationsª *¢ 
,
serving_default"
signature_map
l
_feature_name_to_idx
	_init_ops
#categorical_str_to_int_hashmaps"
_generic_user_object
S
_model_loader
_create_resource
 _initialize
!_destroy_resourceR 
'
0"
trackable_list_wrapper
 "
trackable_list_wrapper
 "
trackable_list_wrapper
 "
trackable_list_wrapper
 "
trackable_dict_wrapper
ÁB¾
#__inference_signature_wrapper_16991jet1btagjet1etajet1phijet1ptjet2btagjet2etajet2phijet2ptjet3btagjet3etajet3phijet3ptjet4btagjet4etajet4phijet4pt	leptoneta	leptonphileptonptmbbmissingenergymagnitudemissingenergyphimjjmjjjmjlvmlvmwbbmwwbb"
²
FullArgSpec
args 
varargs
 
varkwjkwargs
defaults
 

kwonlyargs 
kwonlydefaults
 
annotationsª *
 
 "
trackable_dict_wrapper
 "
trackable_list_wrapper
 "
trackable_dict_wrapper
Q
"_output_types
#
_all_files
$
_done_file"
_generic_user_object
±2®
__inference__creator_16996
²
FullArgSpec
args 
varargs
 
varkw
 
defaults
 

kwonlyargs 
kwonlydefaults
 
annotationsª *¢ 
µ2²
__inference__initializer_17004
²
FullArgSpec
args 
varargs
 
varkw
 
defaults
 

kwonlyargs 
kwonlydefaults
 
annotationsª *¢ 
³2°
__inference__destroyer_17009
²
FullArgSpec
args 
varargs
 
varkw
 
defaults
 

kwonlyargs 
kwonlydefaults
 
annotationsª *¢ 
 "
trackable_list_wrapper
C
%0
&1
'2
$3
(4"
trackable_list_wrapper
* 
*
*
*
*Ü
*__inference__build_normalized_inputs_16877­ÿ
¢û

ó
¢ï

ì
ªè

1
jet1btag%"
inputs/jet1btagÿÿÿÿÿÿÿÿÿ
/
jet1eta$!
inputs/jet1etaÿÿÿÿÿÿÿÿÿ
/
jet1phi$!
inputs/jet1phiÿÿÿÿÿÿÿÿÿ
-
jet1pt# 
inputs/jet1ptÿÿÿÿÿÿÿÿÿ
1
jet2btag%"
inputs/jet2btagÿÿÿÿÿÿÿÿÿ
/
jet2eta$!
inputs/jet2etaÿÿÿÿÿÿÿÿÿ
/
jet2phi$!
inputs/jet2phiÿÿÿÿÿÿÿÿÿ
-
jet2pt# 
inputs/jet2ptÿÿÿÿÿÿÿÿÿ
1
jet3btag%"
inputs/jet3btagÿÿÿÿÿÿÿÿÿ
/
jet3eta$!
inputs/jet3etaÿÿÿÿÿÿÿÿÿ
/
jet3phi$!
inputs/jet3phiÿÿÿÿÿÿÿÿÿ
-
jet3pt# 
inputs/jet3ptÿÿÿÿÿÿÿÿÿ
1
jet4btag%"
inputs/jet4btagÿÿÿÿÿÿÿÿÿ
/
jet4eta$!
inputs/jet4etaÿÿÿÿÿÿÿÿÿ
/
jet4phi$!
inputs/jet4phiÿÿÿÿÿÿÿÿÿ
-
jet4pt# 
inputs/jet4ptÿÿÿÿÿÿÿÿÿ
3
	leptoneta&#
inputs/leptonetaÿÿÿÿÿÿÿÿÿ
3
	leptonphi&#
inputs/leptonphiÿÿÿÿÿÿÿÿÿ
1
leptonpt%"
inputs/leptonptÿÿÿÿÿÿÿÿÿ
'
mbb 

inputs/mbbÿÿÿÿÿÿÿÿÿ
M
missingenergymagnitude30
inputs/missingenergymagnitudeÿÿÿÿÿÿÿÿÿ
A
missingenergyphi-*
inputs/missingenergyphiÿÿÿÿÿÿÿÿÿ
'
mjj 

inputs/mjjÿÿÿÿÿÿÿÿÿ
)
mjjj!
inputs/mjjjÿÿÿÿÿÿÿÿÿ
)
mjlv!
inputs/mjlvÿÿÿÿÿÿÿÿÿ
'
mlv 

inputs/mlvÿÿÿÿÿÿÿÿÿ
)
mwbb!
inputs/mwbbÿÿÿÿÿÿÿÿÿ
+
mwwbb"
inputs/mwwbbÿÿÿÿÿÿÿÿÿ
ª "¨	ª¤	
*
jet1btag
jet1btagÿÿÿÿÿÿÿÿÿ
(
jet1eta
jet1etaÿÿÿÿÿÿÿÿÿ
(
jet1phi
jet1phiÿÿÿÿÿÿÿÿÿ
&
jet1pt
jet1ptÿÿÿÿÿÿÿÿÿ
*
jet2btag
jet2btagÿÿÿÿÿÿÿÿÿ
(
jet2eta
jet2etaÿÿÿÿÿÿÿÿÿ
(
jet2phi
jet2phiÿÿÿÿÿÿÿÿÿ
&
jet2pt
jet2ptÿÿÿÿÿÿÿÿÿ
*
jet3btag
jet3btagÿÿÿÿÿÿÿÿÿ
(
jet3eta
jet3etaÿÿÿÿÿÿÿÿÿ
(
jet3phi
jet3phiÿÿÿÿÿÿÿÿÿ
&
jet3pt
jet3ptÿÿÿÿÿÿÿÿÿ
*
jet4btag
jet4btagÿÿÿÿÿÿÿÿÿ
(
jet4eta
jet4etaÿÿÿÿÿÿÿÿÿ
(
jet4phi
jet4phiÿÿÿÿÿÿÿÿÿ
&
jet4pt
jet4ptÿÿÿÿÿÿÿÿÿ
,
	leptoneta
	leptonetaÿÿÿÿÿÿÿÿÿ
,
	leptonphi
	leptonphiÿÿÿÿÿÿÿÿÿ
*
leptonpt
leptonptÿÿÿÿÿÿÿÿÿ
 
mbb
mbbÿÿÿÿÿÿÿÿÿ
F
missingenergymagnitude,)
missingenergymagnitudeÿÿÿÿÿÿÿÿÿ
:
missingenergyphi&#
missingenergyphiÿÿÿÿÿÿÿÿÿ
 
mjj
mjjÿÿÿÿÿÿÿÿÿ
"
mjjj
mjjjÿÿÿÿÿÿÿÿÿ
"
mjlv
mjlvÿÿÿÿÿÿÿÿÿ
 
mlv
mlvÿÿÿÿÿÿÿÿÿ
"
mwbb
mwbbÿÿÿÿÿÿÿÿÿ
$
mwwbb
mwwbbÿÿÿÿÿÿÿÿÿ6
__inference__creator_16996¢

¢ 
ª " 8
__inference__destroyer_17009¢

¢ 
ª " >
__inference__initializer_17004$¢

¢ 
ª " 

 __inference__wrapped_model_16181ö	»	¢·	
¯	¢«	
¨	ª¤	
*
jet1btag
jet1btagÿÿÿÿÿÿÿÿÿ
(
jet1eta
jet1etaÿÿÿÿÿÿÿÿÿ
(
jet1phi
jet1phiÿÿÿÿÿÿÿÿÿ
&
jet1pt
jet1ptÿÿÿÿÿÿÿÿÿ
*
jet2btag
jet2btagÿÿÿÿÿÿÿÿÿ
(
jet2eta
jet2etaÿÿÿÿÿÿÿÿÿ
(
jet2phi
jet2phiÿÿÿÿÿÿÿÿÿ
&
jet2pt
jet2ptÿÿÿÿÿÿÿÿÿ
*
jet3btag
jet3btagÿÿÿÿÿÿÿÿÿ
(
jet3eta
jet3etaÿÿÿÿÿÿÿÿÿ
(
jet3phi
jet3phiÿÿÿÿÿÿÿÿÿ
&
jet3pt
jet3ptÿÿÿÿÿÿÿÿÿ
*
jet4btag
jet4btagÿÿÿÿÿÿÿÿÿ
(
jet4eta
jet4etaÿÿÿÿÿÿÿÿÿ
(
jet4phi
jet4phiÿÿÿÿÿÿÿÿÿ
&
jet4pt
jet4ptÿÿÿÿÿÿÿÿÿ
,
	leptoneta
	leptonetaÿÿÿÿÿÿÿÿÿ
,
	leptonphi
	leptonphiÿÿÿÿÿÿÿÿÿ
*
leptonpt
leptonptÿÿÿÿÿÿÿÿÿ
 
mbb
mbbÿÿÿÿÿÿÿÿÿ
F
missingenergymagnitude,)
missingenergymagnitudeÿÿÿÿÿÿÿÿÿ
:
missingenergyphi&#
missingenergyphiÿÿÿÿÿÿÿÿÿ
 
mjj
mjjÿÿÿÿÿÿÿÿÿ
"
mjjj
mjjjÿÿÿÿÿÿÿÿÿ
"
mjlv
mjlvÿÿÿÿÿÿÿÿÿ
 
mlv
mlvÿÿÿÿÿÿÿÿÿ
"
mwbb
mwbbÿÿÿÿÿÿÿÿÿ
$
mwwbb
mwwbbÿÿÿÿÿÿÿÿÿ
ª "3ª0
.
output_1"
output_1ÿÿÿÿÿÿÿÿÿ¾
__inference_call_16949£¢ÿ

÷
¢ó

ì
ªè

1
jet1btag%"
inputs/jet1btagÿÿÿÿÿÿÿÿÿ
/
jet1eta$!
inputs/jet1etaÿÿÿÿÿÿÿÿÿ
/
jet1phi$!
inputs/jet1phiÿÿÿÿÿÿÿÿÿ
-
jet1pt# 
inputs/jet1ptÿÿÿÿÿÿÿÿÿ
1
jet2btag%"
inputs/jet2btagÿÿÿÿÿÿÿÿÿ
/
jet2eta$!
inputs/jet2etaÿÿÿÿÿÿÿÿÿ
/
jet2phi$!
inputs/jet2phiÿÿÿÿÿÿÿÿÿ
-
jet2pt# 
inputs/jet2ptÿÿÿÿÿÿÿÿÿ
1
jet3btag%"
inputs/jet3btagÿÿÿÿÿÿÿÿÿ
/
jet3eta$!
inputs/jet3etaÿÿÿÿÿÿÿÿÿ
/
jet3phi$!
inputs/jet3phiÿÿÿÿÿÿÿÿÿ
-
jet3pt# 
inputs/jet3ptÿÿÿÿÿÿÿÿÿ
1
jet4btag%"
inputs/jet4btagÿÿÿÿÿÿÿÿÿ
/
jet4eta$!
inputs/jet4etaÿÿÿÿÿÿÿÿÿ
/
jet4phi$!
inputs/jet4phiÿÿÿÿÿÿÿÿÿ
-
jet4pt# 
inputs/jet4ptÿÿÿÿÿÿÿÿÿ
3
	leptoneta&#
inputs/leptonetaÿÿÿÿÿÿÿÿÿ
3
	leptonphi&#
inputs/leptonphiÿÿÿÿÿÿÿÿÿ
1
leptonpt%"
inputs/leptonptÿÿÿÿÿÿÿÿÿ
'
mbb 

inputs/mbbÿÿÿÿÿÿÿÿÿ
M
missingenergymagnitude30
inputs/missingenergymagnitudeÿÿÿÿÿÿÿÿÿ
A
missingenergyphi-*
inputs/missingenergyphiÿÿÿÿÿÿÿÿÿ
'
mjj 

inputs/mjjÿÿÿÿÿÿÿÿÿ
)
mjjj!
inputs/mjjjÿÿÿÿÿÿÿÿÿ
)
mjlv!
inputs/mjlvÿÿÿÿÿÿÿÿÿ
'
mlv 

inputs/mlvÿÿÿÿÿÿÿÿÿ
)
mwbb!
inputs/mwbbÿÿÿÿÿÿÿÿÿ
+
mwwbb"
inputs/mwwbbÿÿÿÿÿÿÿÿÿ
p 
ª "ÿÿÿÿÿÿÿÿÿ¿

N__inference_random_forest_model_layer_call_and_return_conditional_losses_16535ì	¿	¢»	
³	¢¯	
¨	ª¤	
*
jet1btag
jet1btagÿÿÿÿÿÿÿÿÿ
(
jet1eta
jet1etaÿÿÿÿÿÿÿÿÿ
(
jet1phi
jet1phiÿÿÿÿÿÿÿÿÿ
&
jet1pt
jet1ptÿÿÿÿÿÿÿÿÿ
*
jet2btag
jet2btagÿÿÿÿÿÿÿÿÿ
(
jet2eta
jet2etaÿÿÿÿÿÿÿÿÿ
(
jet2phi
jet2phiÿÿÿÿÿÿÿÿÿ
&
jet2pt
jet2ptÿÿÿÿÿÿÿÿÿ
*
jet3btag
jet3btagÿÿÿÿÿÿÿÿÿ
(
jet3eta
jet3etaÿÿÿÿÿÿÿÿÿ
(
jet3phi
jet3phiÿÿÿÿÿÿÿÿÿ
&
jet3pt
jet3ptÿÿÿÿÿÿÿÿÿ
*
jet4btag
jet4btagÿÿÿÿÿÿÿÿÿ
(
jet4eta
jet4etaÿÿÿÿÿÿÿÿÿ
(
jet4phi
jet4phiÿÿÿÿÿÿÿÿÿ
&
jet4pt
jet4ptÿÿÿÿÿÿÿÿÿ
,
	leptoneta
	leptonetaÿÿÿÿÿÿÿÿÿ
,
	leptonphi
	leptonphiÿÿÿÿÿÿÿÿÿ
*
leptonpt
leptonptÿÿÿÿÿÿÿÿÿ
 
mbb
mbbÿÿÿÿÿÿÿÿÿ
F
missingenergymagnitude,)
missingenergymagnitudeÿÿÿÿÿÿÿÿÿ
:
missingenergyphi&#
missingenergyphiÿÿÿÿÿÿÿÿÿ
 
mjj
mjjÿÿÿÿÿÿÿÿÿ
"
mjjj
mjjjÿÿÿÿÿÿÿÿÿ
"
mjlv
mjlvÿÿÿÿÿÿÿÿÿ
 
mlv
mlvÿÿÿÿÿÿÿÿÿ
"
mwbb
mwbbÿÿÿÿÿÿÿÿÿ
$
mwwbb
mwwbbÿÿÿÿÿÿÿÿÿ
p 
ª "%¢"

0ÿÿÿÿÿÿÿÿÿ
 ¿

N__inference_random_forest_model_layer_call_and_return_conditional_losses_16607ì	¿	¢»	
³	¢¯	
¨	ª¤	
*
jet1btag
jet1btagÿÿÿÿÿÿÿÿÿ
(
jet1eta
jet1etaÿÿÿÿÿÿÿÿÿ
(
jet1phi
jet1phiÿÿÿÿÿÿÿÿÿ
&
jet1pt
jet1ptÿÿÿÿÿÿÿÿÿ
*
jet2btag
jet2btagÿÿÿÿÿÿÿÿÿ
(
jet2eta
jet2etaÿÿÿÿÿÿÿÿÿ
(
jet2phi
jet2phiÿÿÿÿÿÿÿÿÿ
&
jet2pt
jet2ptÿÿÿÿÿÿÿÿÿ
*
jet3btag
jet3btagÿÿÿÿÿÿÿÿÿ
(
jet3eta
jet3etaÿÿÿÿÿÿÿÿÿ
(
jet3phi
jet3phiÿÿÿÿÿÿÿÿÿ
&
jet3pt
jet3ptÿÿÿÿÿÿÿÿÿ
*
jet4btag
jet4btagÿÿÿÿÿÿÿÿÿ
(
jet4eta
jet4etaÿÿÿÿÿÿÿÿÿ
(
jet4phi
jet4phiÿÿÿÿÿÿÿÿÿ
&
jet4pt
jet4ptÿÿÿÿÿÿÿÿÿ
,
	leptoneta
	leptonetaÿÿÿÿÿÿÿÿÿ
,
	leptonphi
	leptonphiÿÿÿÿÿÿÿÿÿ
*
leptonpt
leptonptÿÿÿÿÿÿÿÿÿ
 
mbb
mbbÿÿÿÿÿÿÿÿÿ
F
missingenergymagnitude,)
missingenergymagnitudeÿÿÿÿÿÿÿÿÿ
:
missingenergyphi&#
missingenergyphiÿÿÿÿÿÿÿÿÿ
 
mjj
mjjÿÿÿÿÿÿÿÿÿ
"
mjjj
mjjjÿÿÿÿÿÿÿÿÿ
"
mjlv
mjlvÿÿÿÿÿÿÿÿÿ
 
mlv
mlvÿÿÿÿÿÿÿÿÿ
"
mwbb
mwbbÿÿÿÿÿÿÿÿÿ
$
mwwbb
mwwbbÿÿÿÿÿÿÿÿÿ
p
ª "%¢"

0ÿÿÿÿÿÿÿÿÿ
 
N__inference_random_forest_model_layer_call_and_return_conditional_losses_16747°¢ÿ

÷
¢ó

ì
ªè

1
jet1btag%"
inputs/jet1btagÿÿÿÿÿÿÿÿÿ
/
jet1eta$!
inputs/jet1etaÿÿÿÿÿÿÿÿÿ
/
jet1phi$!
inputs/jet1phiÿÿÿÿÿÿÿÿÿ
-
jet1pt# 
inputs/jet1ptÿÿÿÿÿÿÿÿÿ
1
jet2btag%"
inputs/jet2btagÿÿÿÿÿÿÿÿÿ
/
jet2eta$!
inputs/jet2etaÿÿÿÿÿÿÿÿÿ
/
jet2phi$!
inputs/jet2phiÿÿÿÿÿÿÿÿÿ
-
jet2pt# 
inputs/jet2ptÿÿÿÿÿÿÿÿÿ
1
jet3btag%"
inputs/jet3btagÿÿÿÿÿÿÿÿÿ
/
jet3eta$!
inputs/jet3etaÿÿÿÿÿÿÿÿÿ
/
jet3phi$!
inputs/jet3phiÿÿÿÿÿÿÿÿÿ
-
jet3pt# 
inputs/jet3ptÿÿÿÿÿÿÿÿÿ
1
jet4btag%"
inputs/jet4btagÿÿÿÿÿÿÿÿÿ
/
jet4eta$!
inputs/jet4etaÿÿÿÿÿÿÿÿÿ
/
jet4phi$!
inputs/jet4phiÿÿÿÿÿÿÿÿÿ
-
jet4pt# 
inputs/jet4ptÿÿÿÿÿÿÿÿÿ
3
	leptoneta&#
inputs/leptonetaÿÿÿÿÿÿÿÿÿ
3
	leptonphi&#
inputs/leptonphiÿÿÿÿÿÿÿÿÿ
1
leptonpt%"
inputs/leptonptÿÿÿÿÿÿÿÿÿ
'
mbb 

inputs/mbbÿÿÿÿÿÿÿÿÿ
M
missingenergymagnitude30
inputs/missingenergymagnitudeÿÿÿÿÿÿÿÿÿ
A
missingenergyphi-*
inputs/missingenergyphiÿÿÿÿÿÿÿÿÿ
'
mjj 

inputs/mjjÿÿÿÿÿÿÿÿÿ
)
mjjj!
inputs/mjjjÿÿÿÿÿÿÿÿÿ
)
mjlv!
inputs/mjlvÿÿÿÿÿÿÿÿÿ
'
mlv 

inputs/mlvÿÿÿÿÿÿÿÿÿ
)
mwbb!
inputs/mwbbÿÿÿÿÿÿÿÿÿ
+
mwwbb"
inputs/mwwbbÿÿÿÿÿÿÿÿÿ
p 
ª "%¢"

0ÿÿÿÿÿÿÿÿÿ
 
N__inference_random_forest_model_layer_call_and_return_conditional_losses_16819°¢ÿ

÷
¢ó

ì
ªè

1
jet1btag%"
inputs/jet1btagÿÿÿÿÿÿÿÿÿ
/
jet1eta$!
inputs/jet1etaÿÿÿÿÿÿÿÿÿ
/
jet1phi$!
inputs/jet1phiÿÿÿÿÿÿÿÿÿ
-
jet1pt# 
inputs/jet1ptÿÿÿÿÿÿÿÿÿ
1
jet2btag%"
inputs/jet2btagÿÿÿÿÿÿÿÿÿ
/
jet2eta$!
inputs/jet2etaÿÿÿÿÿÿÿÿÿ
/
jet2phi$!
inputs/jet2phiÿÿÿÿÿÿÿÿÿ
-
jet2pt# 
inputs/jet2ptÿÿÿÿÿÿÿÿÿ
1
jet3btag%"
inputs/jet3btagÿÿÿÿÿÿÿÿÿ
/
jet3eta$!
inputs/jet3etaÿÿÿÿÿÿÿÿÿ
/
jet3phi$!
inputs/jet3phiÿÿÿÿÿÿÿÿÿ
-
jet3pt# 
inputs/jet3ptÿÿÿÿÿÿÿÿÿ
1
jet4btag%"
inputs/jet4btagÿÿÿÿÿÿÿÿÿ
/
jet4eta$!
inputs/jet4etaÿÿÿÿÿÿÿÿÿ
/
jet4phi$!
inputs/jet4phiÿÿÿÿÿÿÿÿÿ
-
jet4pt# 
inputs/jet4ptÿÿÿÿÿÿÿÿÿ
3
	leptoneta&#
inputs/leptonetaÿÿÿÿÿÿÿÿÿ
3
	leptonphi&#
inputs/leptonphiÿÿÿÿÿÿÿÿÿ
1
leptonpt%"
inputs/leptonptÿÿÿÿÿÿÿÿÿ
'
mbb 

inputs/mbbÿÿÿÿÿÿÿÿÿ
M
missingenergymagnitude30
inputs/missingenergymagnitudeÿÿÿÿÿÿÿÿÿ
A
missingenergyphi-*
inputs/missingenergyphiÿÿÿÿÿÿÿÿÿ
'
mjj 

inputs/mjjÿÿÿÿÿÿÿÿÿ
)
mjjj!
inputs/mjjjÿÿÿÿÿÿÿÿÿ
)
mjlv!
inputs/mjlvÿÿÿÿÿÿÿÿÿ
'
mlv 

inputs/mlvÿÿÿÿÿÿÿÿÿ
)
mwbb!
inputs/mwbbÿÿÿÿÿÿÿÿÿ
+
mwwbb"
inputs/mwwbbÿÿÿÿÿÿÿÿÿ
p
ª "%¢"

0ÿÿÿÿÿÿÿÿÿ
 

3__inference_random_forest_model_layer_call_fn_16289ß	¿	¢»	
³	¢¯	
¨	ª¤	
*
jet1btag
jet1btagÿÿÿÿÿÿÿÿÿ
(
jet1eta
jet1etaÿÿÿÿÿÿÿÿÿ
(
jet1phi
jet1phiÿÿÿÿÿÿÿÿÿ
&
jet1pt
jet1ptÿÿÿÿÿÿÿÿÿ
*
jet2btag
jet2btagÿÿÿÿÿÿÿÿÿ
(
jet2eta
jet2etaÿÿÿÿÿÿÿÿÿ
(
jet2phi
jet2phiÿÿÿÿÿÿÿÿÿ
&
jet2pt
jet2ptÿÿÿÿÿÿÿÿÿ
*
jet3btag
jet3btagÿÿÿÿÿÿÿÿÿ
(
jet3eta
jet3etaÿÿÿÿÿÿÿÿÿ
(
jet3phi
jet3phiÿÿÿÿÿÿÿÿÿ
&
jet3pt
jet3ptÿÿÿÿÿÿÿÿÿ
*
jet4btag
jet4btagÿÿÿÿÿÿÿÿÿ
(
jet4eta
jet4etaÿÿÿÿÿÿÿÿÿ
(
jet4phi
jet4phiÿÿÿÿÿÿÿÿÿ
&
jet4pt
jet4ptÿÿÿÿÿÿÿÿÿ
,
	leptoneta
	leptonetaÿÿÿÿÿÿÿÿÿ
,
	leptonphi
	leptonphiÿÿÿÿÿÿÿÿÿ
*
leptonpt
leptonptÿÿÿÿÿÿÿÿÿ
 
mbb
mbbÿÿÿÿÿÿÿÿÿ
F
missingenergymagnitude,)
missingenergymagnitudeÿÿÿÿÿÿÿÿÿ
:
missingenergyphi&#
missingenergyphiÿÿÿÿÿÿÿÿÿ
 
mjj
mjjÿÿÿÿÿÿÿÿÿ
"
mjjj
mjjjÿÿÿÿÿÿÿÿÿ
"
mjlv
mjlvÿÿÿÿÿÿÿÿÿ
 
mlv
mlvÿÿÿÿÿÿÿÿÿ
"
mwbb
mwbbÿÿÿÿÿÿÿÿÿ
$
mwwbb
mwwbbÿÿÿÿÿÿÿÿÿ
p 
ª "ÿÿÿÿÿÿÿÿÿ

3__inference_random_forest_model_layer_call_fn_16463ß	¿	¢»	
³	¢¯	
¨	ª¤	
*
jet1btag
jet1btagÿÿÿÿÿÿÿÿÿ
(
jet1eta
jet1etaÿÿÿÿÿÿÿÿÿ
(
jet1phi
jet1phiÿÿÿÿÿÿÿÿÿ
&
jet1pt
jet1ptÿÿÿÿÿÿÿÿÿ
*
jet2btag
jet2btagÿÿÿÿÿÿÿÿÿ
(
jet2eta
jet2etaÿÿÿÿÿÿÿÿÿ
(
jet2phi
jet2phiÿÿÿÿÿÿÿÿÿ
&
jet2pt
jet2ptÿÿÿÿÿÿÿÿÿ
*
jet3btag
jet3btagÿÿÿÿÿÿÿÿÿ
(
jet3eta
jet3etaÿÿÿÿÿÿÿÿÿ
(
jet3phi
jet3phiÿÿÿÿÿÿÿÿÿ
&
jet3pt
jet3ptÿÿÿÿÿÿÿÿÿ
*
jet4btag
jet4btagÿÿÿÿÿÿÿÿÿ
(
jet4eta
jet4etaÿÿÿÿÿÿÿÿÿ
(
jet4phi
jet4phiÿÿÿÿÿÿÿÿÿ
&
jet4pt
jet4ptÿÿÿÿÿÿÿÿÿ
,
	leptoneta
	leptonetaÿÿÿÿÿÿÿÿÿ
,
	leptonphi
	leptonphiÿÿÿÿÿÿÿÿÿ
*
leptonpt
leptonptÿÿÿÿÿÿÿÿÿ
 
mbb
mbbÿÿÿÿÿÿÿÿÿ
F
missingenergymagnitude,)
missingenergymagnitudeÿÿÿÿÿÿÿÿÿ
:
missingenergyphi&#
missingenergyphiÿÿÿÿÿÿÿÿÿ
 
mjj
mjjÿÿÿÿÿÿÿÿÿ
"
mjjj
mjjjÿÿÿÿÿÿÿÿÿ
"
mjlv
mjlvÿÿÿÿÿÿÿÿÿ
 
mlv
mlvÿÿÿÿÿÿÿÿÿ
"
mwbb
mwbbÿÿÿÿÿÿÿÿÿ
$
mwwbb
mwwbbÿÿÿÿÿÿÿÿÿ
p
ª "ÿÿÿÿÿÿÿÿÿÛ
3__inference_random_forest_model_layer_call_fn_16641£¢ÿ

÷
¢ó

ì
ªè

1
jet1btag%"
inputs/jet1btagÿÿÿÿÿÿÿÿÿ
/
jet1eta$!
inputs/jet1etaÿÿÿÿÿÿÿÿÿ
/
jet1phi$!
inputs/jet1phiÿÿÿÿÿÿÿÿÿ
-
jet1pt# 
inputs/jet1ptÿÿÿÿÿÿÿÿÿ
1
jet2btag%"
inputs/jet2btagÿÿÿÿÿÿÿÿÿ
/
jet2eta$!
inputs/jet2etaÿÿÿÿÿÿÿÿÿ
/
jet2phi$!
inputs/jet2phiÿÿÿÿÿÿÿÿÿ
-
jet2pt# 
inputs/jet2ptÿÿÿÿÿÿÿÿÿ
1
jet3btag%"
inputs/jet3btagÿÿÿÿÿÿÿÿÿ
/
jet3eta$!
inputs/jet3etaÿÿÿÿÿÿÿÿÿ
/
jet3phi$!
inputs/jet3phiÿÿÿÿÿÿÿÿÿ
-
jet3pt# 
inputs/jet3ptÿÿÿÿÿÿÿÿÿ
1
jet4btag%"
inputs/jet4btagÿÿÿÿÿÿÿÿÿ
/
jet4eta$!
inputs/jet4etaÿÿÿÿÿÿÿÿÿ
/
jet4phi$!
inputs/jet4phiÿÿÿÿÿÿÿÿÿ
-
jet4pt# 
inputs/jet4ptÿÿÿÿÿÿÿÿÿ
3
	leptoneta&#
inputs/leptonetaÿÿÿÿÿÿÿÿÿ
3
	leptonphi&#
inputs/leptonphiÿÿÿÿÿÿÿÿÿ
1
leptonpt%"
inputs/leptonptÿÿÿÿÿÿÿÿÿ
'
mbb 

inputs/mbbÿÿÿÿÿÿÿÿÿ
M
missingenergymagnitude30
inputs/missingenergymagnitudeÿÿÿÿÿÿÿÿÿ
A
missingenergyphi-*
inputs/missingenergyphiÿÿÿÿÿÿÿÿÿ
'
mjj 

inputs/mjjÿÿÿÿÿÿÿÿÿ
)
mjjj!
inputs/mjjjÿÿÿÿÿÿÿÿÿ
)
mjlv!
inputs/mjlvÿÿÿÿÿÿÿÿÿ
'
mlv 

inputs/mlvÿÿÿÿÿÿÿÿÿ
)
mwbb!
inputs/mwbbÿÿÿÿÿÿÿÿÿ
+
mwwbb"
inputs/mwwbbÿÿÿÿÿÿÿÿÿ
p 
ª "ÿÿÿÿÿÿÿÿÿÛ
3__inference_random_forest_model_layer_call_fn_16675£¢ÿ

÷
¢ó

ì
ªè

1
jet1btag%"
inputs/jet1btagÿÿÿÿÿÿÿÿÿ
/
jet1eta$!
inputs/jet1etaÿÿÿÿÿÿÿÿÿ
/
jet1phi$!
inputs/jet1phiÿÿÿÿÿÿÿÿÿ
-
jet1pt# 
inputs/jet1ptÿÿÿÿÿÿÿÿÿ
1
jet2btag%"
inputs/jet2btagÿÿÿÿÿÿÿÿÿ
/
jet2eta$!
inputs/jet2etaÿÿÿÿÿÿÿÿÿ
/
jet2phi$!
inputs/jet2phiÿÿÿÿÿÿÿÿÿ
-
jet2pt# 
inputs/jet2ptÿÿÿÿÿÿÿÿÿ
1
jet3btag%"
inputs/jet3btagÿÿÿÿÿÿÿÿÿ
/
jet3eta$!
inputs/jet3etaÿÿÿÿÿÿÿÿÿ
/
jet3phi$!
inputs/jet3phiÿÿÿÿÿÿÿÿÿ
-
jet3pt# 
inputs/jet3ptÿÿÿÿÿÿÿÿÿ
1
jet4btag%"
inputs/jet4btagÿÿÿÿÿÿÿÿÿ
/
jet4eta$!
inputs/jet4etaÿÿÿÿÿÿÿÿÿ
/
jet4phi$!
inputs/jet4phiÿÿÿÿÿÿÿÿÿ
-
jet4pt# 
inputs/jet4ptÿÿÿÿÿÿÿÿÿ
3
	leptoneta&#
inputs/leptonetaÿÿÿÿÿÿÿÿÿ
3
	leptonphi&#
inputs/leptonphiÿÿÿÿÿÿÿÿÿ
1
leptonpt%"
inputs/leptonptÿÿÿÿÿÿÿÿÿ
'
mbb 

inputs/mbbÿÿÿÿÿÿÿÿÿ
M
missingenergymagnitude30
inputs/missingenergymagnitudeÿÿÿÿÿÿÿÿÿ
A
missingenergyphi-*
inputs/missingenergyphiÿÿÿÿÿÿÿÿÿ
'
mjj 

inputs/mjjÿÿÿÿÿÿÿÿÿ
)
mjjj!
inputs/mjjjÿÿÿÿÿÿÿÿÿ
)
mjlv!
inputs/mjlvÿÿÿÿÿÿÿÿÿ
'
mlv 

inputs/mlvÿÿÿÿÿÿÿÿÿ
)
mwbb!
inputs/mwbbÿÿÿÿÿÿÿÿÿ
+
mwwbb"
inputs/mwwbbÿÿÿÿÿÿÿÿÿ
p
ª "ÿÿÿÿÿÿÿÿÿ

#__inference_signature_wrapper_16991ï	´	¢°	
¢ 
¨	ª¤	
*
jet1btag
jet1btagÿÿÿÿÿÿÿÿÿ
(
jet1eta
jet1etaÿÿÿÿÿÿÿÿÿ
(
jet1phi
jet1phiÿÿÿÿÿÿÿÿÿ
&
jet1pt
jet1ptÿÿÿÿÿÿÿÿÿ
*
jet2btag
jet2btagÿÿÿÿÿÿÿÿÿ
(
jet2eta
jet2etaÿÿÿÿÿÿÿÿÿ
(
jet2phi
jet2phiÿÿÿÿÿÿÿÿÿ
&
jet2pt
jet2ptÿÿÿÿÿÿÿÿÿ
*
jet3btag
jet3btagÿÿÿÿÿÿÿÿÿ
(
jet3eta
jet3etaÿÿÿÿÿÿÿÿÿ
(
jet3phi
jet3phiÿÿÿÿÿÿÿÿÿ
&
jet3pt
jet3ptÿÿÿÿÿÿÿÿÿ
*
jet4btag
jet4btagÿÿÿÿÿÿÿÿÿ
(
jet4eta
jet4etaÿÿÿÿÿÿÿÿÿ
(
jet4phi
jet4phiÿÿÿÿÿÿÿÿÿ
&
jet4pt
jet4ptÿÿÿÿÿÿÿÿÿ
,
	leptoneta
	leptonetaÿÿÿÿÿÿÿÿÿ
,
	leptonphi
	leptonphiÿÿÿÿÿÿÿÿÿ
*
leptonpt
leptonptÿÿÿÿÿÿÿÿÿ
 
mbb
mbbÿÿÿÿÿÿÿÿÿ
F
missingenergymagnitude,)
missingenergymagnitudeÿÿÿÿÿÿÿÿÿ
:
missingenergyphi&#
missingenergyphiÿÿÿÿÿÿÿÿÿ
 
mjj
mjjÿÿÿÿÿÿÿÿÿ
"
mjjj
mjjjÿÿÿÿÿÿÿÿÿ
"
mjlv
mjlvÿÿÿÿÿÿÿÿÿ
 
mlv
mlvÿÿÿÿÿÿÿÿÿ
"
mwbb
mwbbÿÿÿÿÿÿÿÿÿ
$
mwwbb
mwwbbÿÿÿÿÿÿÿÿÿ"3ª0
.
output_1"
output_1ÿÿÿÿÿÿÿÿÿL
-__inference_yggdrasil_model_path_tensor_16955$¢

¢ 
ª " 