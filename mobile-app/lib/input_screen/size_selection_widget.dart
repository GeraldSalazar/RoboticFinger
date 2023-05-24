import 'package:flutter/material.dart';

class SizeSelectionWidget extends StatefulWidget {
  final Function(String) onSelectSize;
  SizeSelectionWidget({required this.onSelectSize});
  @override
  _SizeSelectionWidgetState createState() => _SizeSelectionWidgetState();
}

class _SizeSelectionWidgetState extends State<SizeSelectionWidget> {
  String selectedSize = 'Large';

  List<String> sizeOptions = [
    'Small',
    'Medium',
    'Large',
  ];

  void _showSizeOptions() {
    showModalBottomSheet(
      context: context,
      builder: (BuildContext context) {
        return Container(
          child: ListView.builder(
            itemCount: sizeOptions.length,
            itemBuilder: (BuildContext context, int index) {
              final size = sizeOptions[index];
              return Column(
                children: [
                  ListTile(
                    title: Text(size),
                    onTap: () {
                      setState(() {
                        selectedSize = size;
                      });
                      Navigator.pop(context);
                      widget.onSelectSize(selectedSize);
                    },
                  ),
                  const Divider(height: 1.0, color: Colors.grey),
                ],
              );
            },
          ),
        );
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    return Flexible(
      child: GestureDetector(
        onTap: _showSizeOptions,
        child: Container(
          padding: EdgeInsets.all(10.0),
          decoration: BoxDecoration(
            borderRadius: BorderRadius.circular(5.0),
            border: Border.all(color: Colors.grey),
          ),
          child: Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Text(
                selectedSize.isNotEmpty ? selectedSize : 'Select a size',
                style: const TextStyle(fontSize: 16.0),
              ),
              const Icon(Icons.arrow_drop_down),
            ],
          ),
        ),
      ),
    );
  }
}
