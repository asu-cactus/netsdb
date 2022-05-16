import csv
import argparse
import json
import json

from numpy import float64


class Node(object):
    def __init__(self, data):
        self.data = data
        self.left = None
        self.right = None
        self.parent = None


class Tree(object):
    def __init__(self):
        self.root = None
        self.max_depth = None
        self.max_breadth = None

    def create_tree(self, tree, column_names):
        else_check = 0
        node = None
        for index, line in enumerate(tree['Contents']):
            if line.startswith('If'):
                data_str = line[line.find('(') + 1:line.find(')')].replace(' ', '_', 1)
                if len(column_names):
                    data_str = column_names[data_str.split('_')[1].split(' ')[0]] + ' ' + ' '.join(data_str.split('_')[1].split(' ')[1:])
                if not node:
                    node = self.root = Node(data=data_str)
                elif else_check:
                    else_check = 0
                    while node.right:
                        node = node.parent
                    node.right = Node(data=data_str)
                    node.right.parent = node
                    node = node.right
                else:
                    node.left = Node(data=data_str)
                    node.left.parent = node
                    node = node.left
            elif line.startswith('Else'):
                else_check = 1
            elif line.startswith('Predict'):
                if not node:
                    node = self.root = Node(data=float(line.rsplit(' ')[1]))
                elif else_check:
                    else_check = 0
                    while node.right:
                        node = node.parent
                    node.right = Node(data=float(line.rsplit(' ')[1]))
                    node.right.parent = node
                    node = node.parent
                else:
                    node.left = Node(data=float(line.rsplit(' ')[1]))
                    node.left.parent = node
        self.max_depth = self.get_max_depth(self.root) - 1
        self.max_breadth = self.get_max_breadth(self.max_depth)

    def print_inorder(self, node):
        if node is not None:
            self.print_inorder(node.left)
            print(node.data)
            self.print_inorder(node.right)

    def preorder(self, node, node_list=None):
        if node_list is None:
            node_list = []
        if node is not None:
            node_list.append(node)
            self.preorder(node.left, node_list)
            self.preorder(node.right, node_list)
        return node_list

    def get_js_struct(self, node, node_dict=None):
        if node_dict is None:
            node_dict = {'name': node.data, 'children': []}
        if node is not None:
            if node.left:
                new_node_dict_left = {'name': node.left.data, 'type': 'left', 'is_prediction': False, 'children': []}
                node_dict['children'].append(self.get_js_struct(node.left, new_node_dict_left))
            if node.right:
                new_node_dict_right = {'name': node.right.data, 'type': 'right', 'is_prediction': False, 'children': []}
                node_dict['children'].append(self.get_js_struct(node.right, new_node_dict_right))
            else:
                node_dict['is_prediction'] = True
            if node.parent is None:
                node_dict['type'] = 'root'
        return node_dict

    def print_preorder(self, node):
        if node is not None:
            print(node.data)
            self.print_preorder(node.left)
            self.print_preorder(node.right)

    def print_postorder(self, node):
        if node is not None:
            self.print_postorder(node.left)
            self.print_postorder(node.right)
            print(node.data)

    def get_max_depth(self, node):
        if node is None:
            return 0
        else:
            left_depth = self.get_max_depth(node.left)
            right_depth = self.get_max_depth(node.right)
            if left_depth > right_depth:
                return left_depth + 1
            else:
                return right_depth + 1

    def get_max_breadth(self, max_depth=None):
        if max_depth is None:
            max_depth = self.get_max_depth(self.root)
        return 2 ** max_depth


def separate_trees(tree_file):
    tree = ''
    tree_contents = []
    tree_list = []
    for line in tree_file:
        line = line.strip().rstrip()
        if line.find('Tree') != -1:
            if tree:
                tree_list.append({'Tree': tree, 'Contents': tree_contents})
            tree = line
            tree_contents = []
        else:
            tree_contents.append(line)
    tree_list.append({'Tree': tree, 'Contents': tree_contents})
    return tree_list

def read_columns(columns_file):
    column_name_dict = {}
    with open(columns_file) as csvfile:
        reader = csv.DictReader(csvfile)
        for row in reader:
            column_name_dict[row['INDEX']] = row['COLUMN_NAME']
    return column_name_dict


def parse_text(trees_file):
    with open(trees_file) as treesfile:
        threshold = x = [float(i) for i in treesfile.readline().split()]
        numFeatures = int(treesfile.readline().split(',')[3].split('=')[1])
        return threshold, numFeatures, separate_trees(treesfile.readlines())

###############################################################################################################################################33

def add_index_dfs(root, current_index):
    if root is None:
        return

    root['index'] = current_index['index']
    current_index['index'] += 1
    children = root['children']

    for child in children:
        add_index_dfs(child, current_index)

def serialize_tree(root, left, right, features, thresholds, values, number_classes):
    if root is None:
        return

    name = root['name']

    # if not leaf node
    if isinstance(name, str):
        feature_name, threshold_string = name.split('<=')
        feature, feature_num = feature_name.strip().split('_')
    
        features.append(int(feature_num))
        thresholds.append(float(threshold_string))
        values.append([0]) # since we don't care about number of samples with belonging to threshold
    else:
        feature_num = -2
        threshold = -2.0
        class_name = name
        value = [class_name]

        features.append(feature_num)
        thresholds.append(float(threshold))
        values.append(value)


    children = root['children']
    
    # left child
    left_index = -1
    if len(children) > 0:
        left_index = children[0]['index']
    left.append(left_index)

    # right child
    right_index = -1
    if len(children) > 1:
        right_index = children[1]['index']
    right.append(right_index)

    for child in children:
        serialize_tree(child, left, right, features, thresholds, values, number_classes)

def save_to_json(scikit_dict, filename):
    with open(filename, 'w') as fp:
        json.dump(scikit_dict, fp)


def main():
    parser = argparse.ArgumentParser(description='Parse a random forest')
    parser.add_argument('--filepath', dest='trees', help='Path to file holding the trees.', required=True)
    parser.add_argument('--columns', dest='columns', default=None,
                        help='Path to csv file holding column index and column name.')

    args = parser.parse_args()
    column_name_dict = {}
    if args.columns:
        column_name_dict = read_columns(args.columns)
    threshold_values, num_features, trees = parse_text(args.trees)
    tree_list = []
    for index, tree in enumerate(trees):
        tree_obj = Tree()
        tree_obj.create_tree(tree, column_name_dict)
        js_struct = tree_obj.get_js_struct(tree_obj.root)
        node_dict = {'tree': [js_struct], 'max_depth': tree_obj.max_depth, 'max_breadth': tree_obj.max_breadth}
        tree_list.append(node_dict)

    # Meta Data about model
    number_classes = 2
    number_features = num_features
    n_outputs = 1

    max_depth = 0
    for tree in tree_list:
        max_depth = max(tree['max_depth'], max_depth)

    # Populate each decision tree
    estimators = []
    for tree in tree_list:
        left, right, features, threshold, values = [], [], [], [], []
        sampleTree = tree['tree']
        currentIndex = {'index': 0}
        add_index_dfs(sampleTree[0], currentIndex)
        serialize_tree(sampleTree[0], left, right, features, threshold, values, number_classes)

        nodes = []
        values_nested = []
        for i in range (0, len(features)):
            impurity, n_node_samples, weighted_n_node_samples = 0, 0, 0 # not important for prediction
            nodes.append([left[i], right[i], features[i], threshold[i], impurity, n_node_samples, weighted_n_node_samples])
            values_nested.append([values[i]])

        current_dict = {
            'meta': "decision-tree-regression",
            'feature_importances_': [0] * number_features,
            'max_features_': 0, # not sure about it's usage in prediction
            'n_features_': number_features,
            'n_outputs_': n_outputs,
            'tree_': {
                'max_depth': tree['max_depth'],
                'node_count': len(features),
                'nodes': nodes,
                'values': values_nested,
                "nodes_dtype": [ # data types of each entry in nodes i.e left, right, features, threshold, ...
                    "<i8",
                    "<i8",
                    "<i8",
                    "<f8",
                    "<f8",
                    "<i8",
                    "<f8"
                ],
            },
            "params": {

            }
        }
        estimators.append(current_dict)

    scikit_dict = {
        "meta": "gb",
        "classes_": list(range(0, number_classes)),
        "n_classes_": number_classes,
        "max_features_": number_features,
        "n_features_": number_features,
        "train_score_": [1] * len(tree_list),
        "params": {
            "ccp_alpha": 0,
            "criterion": "gini",
            "init": None,
            "learning_rate": 0.1,
            "loss": "deviance",
            "max_depth": tree['max_depth'],
            "max_features": None,
            "max_leaf_nodes": None,
            "min_impurity_decrease": 0.0,
            "min_samples_leaf": 1,
            "min_samples_split": 2,
            "min_weight_fraction_leaf": 0.0,
            "n_estimators": 1,
            "n_iter_no_change": None,
            "random_state": None,
            "subsample": 1.0,
            "tol": 0.0001,
            "validation_fraction": 0.1,
            "verbose": 0,
            "warm_start": False
        },
        "estimators_shape": [
            len(tree_list),
            1 if number_classes == 2 else number_classes
        ],
        "estimators_": estimators,
        "init_": {
            "strategy": "prior",
            "random_state": None,
            "constant": None,
            "_strategy": "prior",
            "sparse_output_": False,
            "n_outputs_": n_outputs,
            "classes_": list(range(0, number_classes)),
            "n_classes_": number_classes,
            "class_prior_": threshold_values,
            "meta": "dummy"
        },
        "loss_": "deviance"
    }

    save_to_json(scikit_dict, 'converted_scikit_model.json')

if __name__ == "__main__":
    main()
    print ('model successfully saved to converted_scikit_model.json')